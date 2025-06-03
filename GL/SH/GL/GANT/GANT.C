#include "cinc.h"
#include <math.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "port.h"
#include "resource.h"
#include "gant.h"

#define msilence    500  // тихий кусок
#define NGR 	   4000  
#define NSAMPLE_IN 8000  // msilence*8

static int SB_FREQ = 4;

#define disprange (20*SB_FREQ)
    // промежуток вычисления бегущей дисперсии

#define nOfSpeakers 4

struct {
  double sdispers,ndispers;
  long nOfSamples;
  long pos;
  DOUBLEARRAY 
       ampS, // усредненная амплитуда
       prob; // вероятность перехода
} ch[nOfSpeakers];

static long nOfSamples = 0;

static DOUBLEARRAY
  rc0   = NULL,     // AutoFilt
  xemm  = NULL,
  sr0   = NULL;
  
static waveBuffer pb;
static BOOL TestOn = FALSE,
     WaveMode = FALSE;

static int  tchannel = 0;

static BOOL stopMode = FALSE; // для остановки локатора

static sprocess gant; // лоцирующий процесс

static long 
  delay100,      // задержки
  msilencedelay = 300;
  

// R S O L V

#define DJ 45.0 // расстояние между динамиками в сантиметрах

#define sq125  1.11803398875 
#define sq149  0.6666666666667

static double        // координаты вершин
   mtr[4][3] = {  { -0.5*DJ,        0.0,       0.0},  
                  {  0.5*DJ,        0.0,       0.0},
                  {  0.0,      sq125*DJ,       0.0},
                  {  0.0,  sq125/3.0*DJ,  sq149*DJ}};

static double xbug[4]; // координаты жука, невязка
static short  ixbug[3]; // координаты в отсчетах

// число сантиметров в одном отсчете:
//   (331.0*1.03 = 340.93,    340.93/11025.0 = 0.030923)

#define CountToMeter (3.0923356/SB_FREQ) 
  

static VOID ClearMid( HWND hWnd ) {
// кнопка Clear.

  int i;
  
  loop( i, nOfSpeakers ) {
    ch[i].nOfSamples = 0;
    
    vequC(ch[i].ampS, 0.0);
    vequC(ch[i].prob, 0.0);
    
    ch[i].pos = 0;
    ch[i].sdispers = 1.0;
    ch[i].ndispers = 1.0;
  }
  nOfSamples = 0;
}
  

BOOL FAR PASCAL __export GantDialogProc(HWND hWnd, unsigned msg, WORD wParam, LONG lParam);


int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int iCmdShow) {
    
  ghInst = hInst;  /* Save instance handle for dialog boxes. */
  ColorsOn(TRUE);
  myDialogBox( NULL, "IDD_GANT", (FARPROC) GantDialogProc);
  ColorsOn(FALSE);
  return TRUE;
}


BOOL GanteliaOn(HWND hWnd, BOOL St) {
  int i;
  
  if ( error ) 
    return FALSE;

  doubleOn(hWnd, &sl, NSAMPLE_IN, St);
  doubleOn(hWnd, &sr, NSAMPLE_IN, St);
  
  loop( i, nOfSpeakers ) {
    doubleOn(hWnd, &(ch[i].ampS), NGR, St);  
    doubleOn(hWnd, &(ch[i].prob), NGR, St);  
  }  
  
  doubleOn(hWnd, &rc0, map, St);       // AutoFilt
  doubleOn(hWnd, &xemm, msilence, St);
  doubleOn(hWnd, &sr0, msilence, St);

  TestOn = FALSE;
  
  waveOn ( hWnd, 2, 2, SB_FREQ, St ); // hWnd, nOfBytes, nOfChannels, frequency, St.

  bufferOn ( hWnd, &pb, NSAMPLE_IN*4L,0, St );

  if ( error ) 
    return FALSE;

  if ( St ) {
	initXemm( xemm );
    pd[0] = ch[0].ampS;
    pd[1] = ch[0].prob;
    ClearMid(hWnd);
    loop (i, 3) {
      ixbug[i] = NGR-200;
      xbug[i] = 3.0;
    }
    xbug[3] = 0.0;  
  }
}


short pauD(DOUBLEARRAY d) {
// поиск смещения щелчка

  short i,im,m;
  double fm,fa;
  
  m = getMemDC(d)->m;
  
  im = absP(d);
  fm = fabs(d[im])*0.1;
  
  for ( i = 1; i < m; i++ ) {
    fa = fabs(d[i]);
    if ( fa > fm ) {
      return i;
    }
  }

  return 0;  
}


BOOL AddMidAmp(HWND hWnd, short nch ) {
// обработка очередного оцифрованного буфера

  short  i,j,i0;
  double s,aa[map+1],ds;
  
  if ( error )
    return 0;

  vtrend(sl);
  vtrend(sr);
  
  i0 = pauD(sl);
  
  if ( i0 < msilence ) 
    return FALSE;
    
  i0 -= msilence;

  if ( i0+NGR > NSAMPLE_IN )
    return FALSE;  

//   Авто - фильтрация
    
  vLowFilter(sr);

  loop(i,msilence)
    sr0[i] = sr[i0+i];
    
  vmul(sr0,sr0,xemm);  

  corr(sr0,rc0);
  
  if ( not autoc(aa,rc0) )
    return FALSE;
    
  loop2 (i,map,NSAMPLE_IN) {
    s = 0.0;
    loop (j, map)
      s += aa[j+1]*sr[i-j];
    
    sr[i-map] = s;
  }
  
  loop2 (i,map,NSAMPLE_IN)
    sr[NSAMPLE_IN-i+map-1] = sr[NSAMPLE_IN-i];
    
// нормализация

  loop (i,NGR) {
    s = sr[i0+i];

    if ( s < 0.0 )
      sr[i0+i] = -log(1.0-s);
    else
      sr[i0+i] = log(1.0+s);
  }       

// шумоподавление
  
  s = 0.0;
  
  loop(i,msilence)
    s += sqr(sr[i0+i]);
  
  s /= msilence;

  if ( s < 0.001 )
    s = 0.001;
  
  ds = 100.0/s;
  
// усреднение  

  ch[tchannel].nOfSamples++;

  loop ( i, NGR ) 
    ch[nch].ampS[i] += sr[i0+i]*ds;

  s = 0.0;
  
  loop(i,msilence)
    s += sqr(ch[nch].ampS[i]);

  ch[nch].sdispers = s/msilence;  

  j = ixbug[nch];

  if ( (j+disprange) >= NGR )
    j = NGR-disprange-1;

  s = 0.0;

  loop(i,disprange)
    s += sqr(ch[nch].ampS[i+j]);
      
  s /= disprange;  
    
  if ( s < ch[nch].sdispers )
    s = ch[nch].sdispers;
 
  ch[nch].ndispers = s;

  return TRUE;
}


static double nev( double xbugn[] ) {
// невязка

  short i,j,k;
  double s,nv,pm;
  
  loop (i,3) {
    if ( xbugn[i] > 5000.0 )
      xbugn[i] = 5000.0;

    if ( xbugn[i] < -5000.0 )
      xbugn[i] = -5000.0;
  }
  
  nv = 0.0;
  
  loop (i,4) {
    s = 0.0;
    
    loop(j,3) 
      s += sqr(mtr[i][j]-xbugn[j]);
      
    j = msilence+round(sqrt(s)/CountToMeter);
    
    if ( j >= NGR-20 )
      j = NGR-20;
      
    ixbug[i] = j;
    
    pm = ch[i].prob[j];
    
    loop (k,20) {
      s = 1.0-k/100.0;
      pm = fmax(pm,ch[i].prob[j+k+1]*s);
      pm = fmax(pm,ch[i].prob[j-k-1]*s);
    }
    
    nv += pm;
  }
  return nv;
}


static BOOL randomStep( double range ) {
// блуждания

  int i,j;
  double vn[3],sn,ds, s;
  BOOL newVal;
  
  s = nev(xbug);
  
  ds = range/RAND_MAX;
  
  newVal = FALSE;
  
  loop(i,200) {     
    loop(j,3) {
      sn = rand();
      vn[j] = xbug[j]-range*0.5+sn*ds;
    }
    sn = nev(vn);
    if (sn > s) {
      loop(j,3)
        xbug[j] = vn[j];
      s = sn;
      newVal = TRUE;
      xbug[3] = s;
    }
  }

  return newVal;  
}


VOID rsolv( void ) {
// аппроксимация координат жука по вычесленным вероятностям перехода

  int i;
  double s;

  loop(i,3)
    xbug[i] = 0.0;

  xbug[3] = 0.0;  
    
  s = 1000.0;

  loop(i,10) {
    while ( randomStep(s) ) ;
    s *= 0.5;
  }  
  s = nev(xbug); // для вычисления ixbug[]
} 


VOID getDistances(HWND hWnd) {// определение координат

  short i,j,
    tch; // переменная цикла по динамикам
  double 
    s,
    dslog, // log(2.0*пи*(дисперсия**2))
    dispSqr, // квадрат дисперсии тишины
    dispSqrD, // dispSqr*2
    tdisp,   // бегущая дисперсия
    tdispa,  // она же, но с проверкой на минимальное значение
    txSqr;   // amp[]**2
    
  DOUBLEARRAY 
    amp,pro; // текущие ampS, prob.

  // определение вероятностей
  
  loop (tch, nOfSpeakers) {
  
    s = 0.0;
    amp = ch[tch].ampS;
    pro = ch[tch].prob;
  
    loop ( i, msilence ) 
      s += sqr(amp[i]);

    dispSqr = fmax ( s/msilence, 0.0001 );

    if (dispSqr < 0.00011) 
      message(hWnd,"Small dispSqr");
    
    ch[tch].sdispers = dispSqr; 
    
    // проход вперед
    
    s = 0.0;
    
    dslog = 0.5*log((6.283*dispSqr));
    
    dispSqrD = dispSqr*2.0;
  
    loop ( i, NGR ) {
      s = s-dslog-sqr(amp[i])/dispSqrD;
      pro[i] = s;
    }
    
    // теперь назад
    
    s = 0.0;

    tdisp = 0.0;
    dispSqrD = dispSqr*1.4;

    loop ( i, NGR ) {
      j = NGR-i-1;
      
      txSqr = sqr(amp[j]);
      
      tdisp += txSqr;
      
      if ( (j+disprange) < NGR )
        tdisp -= sqr(amp[j+disprange]);
        
      tdispa = tdisp/disprange;
      
      if ( tdispa < dispSqrD )   
        tdispa = dispSqrD;

      pro[j] += s;

      s = s-0.5*log(6.283*tdispa)-txSqr/(2.0*tdispa);
    }  
    
    s = pro[0];
    
    loop ( i, NGR ) {
      pro[i] -= s;
      if ( pro[i] < 0.0 )
        pro[i] = 0.0;
    }
  }
  
  // определение координат
  
  rsolv();
  
  // определение отношения дисперсий
  
  loop(tch, nOfSpeakers) {
    j = ixbug[tch];
    amp = ch[tch].ampS;

    if ( (j+disprange) >= NGR )
      j = NGR-disprange-1;

    s = 0.0;
    loop(i,disprange)
      s += sqr(amp[i+j]);
      
    s /= disprange;  
    
    if ( s < ch[tch].sdispers )
      s = ch[tch].sdispers;
    ch[tch].ndispers = s;
  }
}

  
VOID drawDir(HDC rDC, RECT *rF) {
// рисование жука

  HBRUSH  hOldBrush;
  HPEN  hOldPen;
  short x0,y0,dx,dy,lt;
  DWORD dwColor,dwBColor,dwExt;
  char s[128];

  hOldBrush = SelectObject(rDC, pen.bGray);

  Rectangle(rDC,rF->left,rF->top,rF->right,rF->bottom);

  SelectObject(rDC, hOldBrush);
  
  x0 = (rF->left+rF->right)/2;
  y0 = (rF->top+rF->bottom)/2;
  dx = (rF->right-rF->left-4)/2;
  dy = (rF->bottom-rF->top-4)/2;
  
  if (dx < 1)
    dx = 1;
  if (dy < 1)
    dy = 1;  
  
  hOldPen = SelectObject(rDC, GetStockObject(WHITE_PEN));
  
  MoveTo(rDC,x0-dx,y0);
  LineTo(rDC,x0+dx,y0);

  MoveTo(rDC,x0,y0-dy);
  LineTo(rDC,x0,y0-dy+20);

  MoveTo(rDC,x0,y0+dy-20);
  LineTo(rDC,x0,y0+dy);

  { 
    double r1,r2,r3,dxV,dyV;
    short xV,yV,dRx,dRy,i;
    
    loop ( i, 10 ) {
      r1 = 2*i+1;
      r1 = log(r1+1.0)/log(21.0);  
    
      dRx = round(dx*r1);
      dRy = round(dy*r1);
      
      MoveTo(rDC,x0-dRx,y0+5);
  	  LineTo(rDC,x0-dRx,y0-5);

      MoveTo(rDC,x0+dRx,y0+5);
  	  LineTo(rDC,x0+dRx,y0-5);
    }

    if ( FALSE ) {
      dwColor = SetTextColor(rDC, RGB(255,255,0));
      SelectObject(rDC, pen.pGreen);
    }
    else {  
      dwColor = SetTextColor(rDC, RGB(0,255,255));
      SelectObject(rDC, pen.pHGreen);
    }

    r1 = sqrt(sqr(xbug[0])+sqr(xbug[1])+sqr(xbug[2]))*0.01;
    r2 = r1;

    sprintf(s,"Distance =%5.2f m", r1);
    
    if ( r1 > 2000.0 ) 
      r1 = 2000.0;
      
    r1 = log(r1*0.01+1.0)/log(21.0);  
    
    if (xbug[0] < 0.0) {
      dxV = -xbug[0]*0.01;
      if ( dxV > 2000.0 )
        dxV = 2000.0;
      dxV = -log(1.0+dxV)/log(21.0);
    }
    else {
      dxV = xbug[0]*0.01;
      if ( dxV > 20.0 )
        dxV = 20.0;
      dxV = log(1.0+dxV)/log(21.0);
    }

    if (xbug[1] < 0.0) {
      dyV = -xbug[1]*0.01;
      if ( dyV > 20.0 )
        dyV = 20.0;
      dyV = -log(1.0+dyV)/log(21.0);
    }
    else {
      dyV = xbug[1]*0.01;
      if ( dyV > 20.0 )
        dyV = 20.0;
      dyV = log(1.0+dyV)/log(21.0);
    }
    
    xV = round(dxV*dx);
    yV = round(dyV*dy);

    if ( TRUE ) {
      SelectObject(rDC, pen.hDashPen);
    }
    else {  
      SelectObject(rDC, pen.hDotPen);
    }

    MoveTo(rDC,x0+xV,y0-yV);
    LineTo(rDC,x0,y0);

    dwBColor = SetBkColor(rDC, RGB(128,128,128));
    TextOut(rDC, x0-dx+5, y0-28, "Left", strlen("Left"));

    dwExt = GetTextExtent (rDC, (LPCSTR)"Right", strlen("Right"));
  
    lt = LOWORD(dwExt);
    
    TextOut (rDC, x0+dx-lt-5, y0-28, "Right", strlen ("Right"));

    dwExt = GetTextExtent (rDC, (LPCSTR)s, strlen(s));
  
    lt = LOWORD(dwExt);
  
    TextOut (rDC, x0+dx-lt-10, y0+dy/2-10, s, strlen(s));
    
    if ( r2 < 0.01 ) 
      r2 = 0.01;
    
    r3 = xbug[2]*0.01;
    
    if ( r3 > 0 ) {
      if ( r3 > r2 ) 
        r3 = r2;
    }
    else {  
      if ( r3 < r2 ) 
        r3 = r2;
    }
             
    sprintf(s,"HA =%5.0f°", asin(r3/r2)*180.0/3.1415926);
  
    TextOut (rDC, x0+dx-lt-10, y0+HIWORD(dwExt)+dy/2-10, s, strlen(s));
    
    sprintf(s,"P =%3.0f", xbug[3]*0.001);

    lt = HIWORD(dwExt);
  
    TextOut (rDC, x0-dx+10, y0+lt+dy/2-10,s,strlen(s));

  }
  SetTextColor(rDC, dwColor);
  SetBkColor(rDC, dwBColor);

  SelectObject(rDC, hOldPen);
}
  

VOID DrawDirs( HWND hWnd ) {
// вызов рисования жука

  HDC hdc;
  HWND hdlg;
  RECT rf;
    
  hdlg = GetDlgItem(hWnd,IDC_PICTURE);
  hdc = GetDC(hdlg);
  
  GetClientRect(hdlg,&rf);
  
  drawDir(hdc,&rf);
  
  ReleaseDC(hWnd,hdc);
}


static VOID statReport(HDC hDC) {
// выдача сообщений о текущей частоте в окно состояния программы
// (перерисовка окна со статистикой).

  TEXTMETRIC textmetric;
  int nDrawX;
  int nDrawY;
  char szText[300];
    
  sprintf(szText,
        "d = %5.0f, %5.0f, %5.0f, %5.0f.", xbug[0], xbug[1], xbug[2], xbug[3]);  

  GetTextMetrics (hDC, &textmetric);

  nDrawX = GetDeviceCaps (hDC, LOGPIXELSX) / 4;   
  nDrawY = GetDeviceCaps (hDC, LOGPIXELSY) / 4;   // 1/4 inch 
  
  SetBkColor(hDC,RGB(128,128,128));

  TextOut (hDC, nDrawX, nDrawY, szText, strlen (szText));
/*
  nDrawY += textmetric.tmExternalLeading + textmetric.tmHeight;

  sprintf(szText,
        "s = (%5.0f, %5.0f, %5.0f), %6.0f, N of sampl %ld.", 
        v[0],v[1],v[2],v[3], nOfSamples); 

  TextOut (hDC, nDrawX, nDrawY, szText, strlen (szText));
*/
}


static VOID statDistPut( void ) {
// перерисовка окна со статистикой

  HDC hDC;
  
  if ( pFunWin.hWnd != NULL ) {
  hDC = GetDC(pFunWin.hWnd);
  statReport(hDC);
  ReleaseDC(pFunWin.hWnd,hDC);
  }
}


static VOID StartWaveIn( HWND hWnd ) {
// запуск оцифровки

  if (WaveMode or error) 
    return;
  
  waveAddBuffer( hWnd, &pb );
  WaveMode = TRUE;
  waveStart(hWnd);
}


short getMinChannel( void ) {
// возвращяет наиболее подходящий для стука динамик

  short i, im;
  double sm, ts;
  
  loop (i, nOfSpeakers ) {
    if ( ch[i].nOfSamples < 1 )
      return i;
  }

  im = 0;
  sm = 0.0;
  loop (i, nOfSpeakers ) {
    ts = ch[i].sdispers/ch[i].ndispers;
    if ( ts > sm ) {
      im = i;
      sm = ts;
    }  
  }
  return im;
}


static VOID GantProcess(HWND hWnd) {
// (акустический процесс).
  short i;
  
  for (;;) 
  switch ( gant.state ) {
    case 100:  //  начало работы - измерение времени delay(100)
      setDirection(hWnd,waveInp);
      delay(100);   // (в отсчетах оцифровки).
      StartWaveIn(hWnd);
      delay(10);
      outport(5);
      exitstate(gant,200);
      
    case 200:
      delay100 = pauD(sl);
      if ( delay100 < 10 ) {
    	message(hWnd,"err in delay");
        error = TRUE;
        exitstate(gant,0);
      }
      delay(100);
      StartWaveIn(hWnd);
      delay(110);
      outport(5);
      exitstate(gant,300);
      
    case 300:
      i = pauD(sl);
      if ( i < delay100+10 ) {
        errormessage(hWnd,"delay");
        exitstate(gant,0);
      }

      delay100 = i-delay100;
      msilencedelay = ((msilence+10L)*100L+msilence*20L)/delay100+20L;
      if ( msilencedelay < 30 ) {
        errormessage(hWnd,"slow");
        exitstate(gant,0);
      }

// начало локации

      delay(msilencedelay);

    case 400:
      tchannel = getMinChannel();

    case 450:
      StartWaveIn(hWnd);
      delay(msilencedelay);
      outport(tchannel+1);
      delay(msilencedelay*5);
      exitstate(gant,500);
      
    case 500:
      if ( !AddMidAmp(hWnd,tchannel) )
        gotostate(gant,450);
      pd[0] = ch[tchannel].ampS;
      pd[1] = ch[tchannel].prob;
      putpFun(0);
      putpFun(1);
  
      nOfSamples++;
      
      if ( stopMode ) {
        TestOn = FALSE;
        setDirection(hWnd,waveNull);
        printW(hWnd,IDC_START,"Start");
        stopMode = FALSE;
        exitstate(gant,0);
      }  
            
      if ( (nOfSamples % 10) != 0 )
        gotostate(gant,400);
      
      getDistances(hWnd);
      
      DrawDirs(hWnd);
      statDistPut();

      gotostate(gant,400);  
      
    default:
      message(hWnd, "acu proc state error");
      error = TRUE;
      exitstate(gant,0);
      break;
  }
}


VOID ShoWin( HWND hWnd ) {
// порождение дополнительных окон

  RECT rW;
  short dx,dy;
  
  GetWindowRect(hWnd,&rW);
  
  dy = (GetSystemMetrics(SM_CYSCREEN)-rW.bottom);
  dx = GetSystemMetrics(SM_CXSCREEN);

  makeWaveWin(hWnd,rW.right,0,dx-rW.right,rW.bottom);
  	
  if ( pFunWin.hWnd == NULL )
    makepFunWin(hWnd, 0, rW.bottom, dx, dy);
}


BOOL FAR PASCAL __export GantDialogProc(HWND hWnd, unsigned msg, WORD wParam, LONG lParam)
// основная оконная функция (диалог) 
{   
  if ( error ) {
    if ( msg == WM_COMMAND ) {
      if ( (wParam == IDOK) or (wParam == IDCANCEL) )
        EndDialog(hWnd,1) ;
    }    
    return FALSE;
  }  
  
  switch (msg) {
    case WM_SYSCOMMAND:
        switch (wParam)
        {
	    case IDC_OPTIONS:
          ShoWin(hWnd);
          return TRUE; 
        
        default :
          return FALSE;         
        }
        break;

    case WM_COMMAND:
      switch (wParam) {
      
        case IDOK:
        case IDCANCEL:
            if ( TestOn == FALSE )
              EndDialog(hWnd,1) ;
            break;
            
        case IDC_CLEAR:
            ClearMid( hWnd );    	
    	    break;

    	case IDC_START:
            if (!TestOn) {
              stopMode = FALSE;
              TestOn = TRUE;
              gant.state = 100;
              GantProcess(hWnd);
              printW(hWnd,IDC_START,"Stop");
            }  
            else 
              stopMode = TRUE;
  	        break;
      }
      break;
         
    case MM_WIM_DATA: 		// End Of Record.        
         WaveMode = FALSE;
         if ( waveDone(hWnd,lParam) != 0 ) {
           errormessage(hWnd, "MM_WIM_DONE buffer state error");
		   return TRUE;        
         }
         copyWave(&pb);
         putWave();
         if ( gant.state > 5 )
           GantProcess(hWnd);  
         break;
         
    case WM_INITDIALOG:
         AppendMenu(GetSystemMenu(hWnd, NULL),
           MF_STRING | MF_ENABLED, IDC_OPTIONS, "Windows...");
         GanteliaOn(hWnd,TRUE);
         ShoWin(hWnd);
         TestOn = TRUE;
         gant.state = 100;
         GantProcess(hWnd);
         break; 
         
	case WM_DESTROY:
	     GanteliaOn(hWnd,FALSE);
         break;
    default :
         return FALSE;         
    }
    return TRUE;
}

