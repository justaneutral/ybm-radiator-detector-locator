#include "cinc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fcomm.h"
#include "scan.h"
#include "port.h"
#include "resource.h"

// акустическая проверка

#define msample   7000

static BOOL contScan = FALSE,xmode = FALSE;

// счетчики
static long currenttime = 0; // счетчик времени
static short nofcircles = 5; // число проходов по частотам
static short nofsounds = 1;  // минимальное число стуков на радиостанцию
static short checkbuzzloop = 0;  // число стуков

// статистика

static long scancirclecount = 0; // число проходов циклического сканирования
static fint checkfrequency = 0;  // для отчета об акустической проверке.
static double checkpower = 0.0;

static struct pitchtabtag pitch = {0,0,0,NULL,NULL}; 

static waveBuffer pb;

// Автомат сканирования

static fint circlefreq;

static short dcirclecount = 0;  // счетчик для отслеживания начального смещения
                                // частот в циклическом сканировании
                                
static short dcircle[5] = {0,3000,1000,4000,2000};

static vint  pvradio = 0; // Предыдущее значение сигнала в текущей точке
                       // (для ожидания, пока она стабилизируется)
                       
static BOOL  emptyvradio = TRUE; // признак, пустого pvradio

fint   bigstep    = 5000, // шаг сканирования при отсутствии сигнала
       smallstep  =  250; // шаг сканирования в присутствии сигнала

BOOL tryAgainOnCommError = FALSE; // реакция на ошибку Comm

static fpoint currentscalepoint, // для рисования шкалы сканирования
		   leftpoint,rightpoint, // левая / правая границы
           leftmx,rightmx;       // левый / правый максимум

fpoint currentpoint; // для передачи параметров (текущая точка сканирования)

// main win

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);

sprocess acu,scan;

static void acuproc( HWND );

// **************************************************************

static BOOL ScaleOn = FALSE; // признак того, что оконный класс уже зарегестрирован.

static struct  {
   HWND          hWnd;  
   RECT           rf,  // общий прямоугольник
                  rfs;  // шкала сканирования
}  fwScale = {NULL};

// **************************************************************
// п/п рисования. 

short ftos( fint f ) {
// вычисления места в шкале сканирования, соответсвщего частоте f.
  fint fdiap,fpos;
  short i,m;
  
  if ( error )
    return 0;
    
  fdiap = pitch.lastfreq-pitch.firstfreq+1;
  if ( fdiap < 1 )
    return 0;
  fpos = currentpoint.f-pitch.firstfreq;
  if ( ( fpos < 0 ) or ( fpos >= fdiap ) )
    return 0;  
  m = pitch.mscale;
  i = round((((double)fpos)*m)/(double)fdiap);
  if ( i >= m )
    i = m-1;   
  if ( i < 0 )
    i = 0;
  return i;  
}


VOID circlefreqReport( void ) {
// изменение шкалы сканирования
// currentpoint содержит текущую частоту и мощность.

  long i;
  
  if ( error )
    return;
    
  i = ftos(currentpoint.f);
    
  if (currentscalepoint.f != i) {
    pitch.radio[currentscalepoint.f].v = currentscalepoint.v;
    pitch.live[currentscalepoint.f] = 5;
    currentscalepoint.f = i;
    currentscalepoint.v = 0;
  }  
  
  if (currentpoint.v > currentscalepoint.v)
    currentscalepoint.v = currentpoint.v;
}


static VOID statReport(HDC hDC) {
// выдача сообщений о текущей частоте в окно состояния программы
// (перерисовка окна со статистикой).

  TEXTMETRIC textmetric;
  int nDrawX;
  int nDrawY;
  char szText[300];
  
  fint tf;
  
  tf = circlefreq+pitch.firstfreq;
  
  if ( tf < pitch.firstfreq )
    tf = pitch.firstfreq;
  if ( tf > pitch.lastfreq )
    tf = pitch.lastfreq;  
    
  sprintf(szText,
  "From  %ld.%04ld mHz  (%ld.%04ld mHz)  to  %ld.%04ld mHz. nChecks %ld.          ",
    (pitch.firstfreq/10000),
    (pitch.firstfreq%10000)/1000,
    (tf/10000),
    (tf%10000)/1000,
    (pitch.lastfreq/10000),
    (pitch.lastfreq%10000)/1000,
    scancirclecount );

  GetTextMetrics (hDC, &textmetric);

  nDrawX = GetDeviceCaps (hDC, LOGPIXELSX) / 4;   /* 1/4 inch */
  nDrawY = GetDeviceCaps (hDC, LOGPIXELSY) / 4;   /* 1/4 inch */
  
  SetBkColor(hDC,RGB(128,128,128));

  TextOut (hDC, nDrawX, nDrawY, szText, strlen (szText));
  nDrawY += textmetric.tmExternalLeading + textmetric.tmHeight;

  sprintf(szText,
  "Check frequency %ld.%04ld mHz : P %7.3f from %d.             ",
    (checkfrequency/10000),
    (checkfrequency%10000)/10,
    checkpower,
    checkbuzzloop);
  TextOut (hDC, nDrawX, nDrawY, szText, strlen (szText));
}


VOID statPut( void ) {
// перерисовка окна со статистикой
  HDC hDC;
  
  hDC = GetDC (fwScale.hWnd);
  statReport(hDC);
  ReleaseDC(fwScale.hWnd,hDC);
}


VOID newCircle( void ) {
// инициализация по пачалу нового прохода циклического сканирования
  short i;

  if ( error )
    return;
    
  pvradio = 0; // Предыдущее значение сигнала в текущей точке
                       // (для ожидания, пока она стабилизируется).
  emptyvradio = TRUE;
  pvradio =  0;
  dcirclecount++;
  if ( dcirclecount > 4 )
    dcirclecount = 0;
  circlefreq = dcircle[dcirclecount];  
  
  loop (i, pitch.mscale) {
    if ( pitch.live[i] > 0 ) {
      pitch.live[i]--;
      
      if ( pitch.live[i] < 1)
        pitch.radio[i].v = 0;
    }
  }  
}

//
//****************** On / Off ***********************
//

static VOID ScanOn( HWND hWnd, BOOL St ) {
// инициализация для акустической части программы
  short j;
  
  LPGLOBALARRAY g;
  
  doubleOn(hWnd, &sl, msample, St);
  doubleOn(hWnd, &sr, msample, St);
  
  g = getMemDC(sl);
  
  waveOn(hWnd,2,2,1,St); // hWnd, nOfBytes,nOfChannels,frequency,St

  bufferOn(hWnd, &pb, msample*4L, 0, St);
            
  // инициализация или освобождение памяти для шкалы частот

  if (St)
    pitch.mscale = GetSystemMetrics(SM_CXSCREEN);
  
  memOn( hWnd, &(pitch.radio), pitch.mscale*sizeof(scalepoint), St);
  
  memOn( hWnd, &(pitch.live), pitch.mscale*sizeof(char), St);
  
  ConnectionOn( hWnd, St );
  
  if ( error ) 
    return;

  if ( St ) {
    outport(0);

    loop (j,pitch.mscale) {
      pitch.radio[j].v  = 0;
      pitch.radio[j].pv = 0;
      pitch.live[j] = 0;
    }  
    
    getMemDC(pitch.radio)->m = pitch.mscale;
    getMemDC(pitch.live)->m = pitch.mscale;

    currenttime = 0; // счетчик времени

    scancirclecount = 0; // число проходов циклического сканирования.

    initproc(&acu);
    initproc(&scan);     

    circlefreq = 0;
    dcirclecount = 0;

    pvradio = 0; // Предыдущее значение сигнала в текущей точке
                    // (для ожидания, пока она стабилизируется).
    emptyvradio = TRUE; // признак пустого pvradio. 
    tryAgainOnCommError = FALSE; // реакция на ошибку Comm
    
    currentscalepoint.f = 0;
    currentscalepoint.v = 0;

    pitch.firstfreq =     1000;
    pitch.lastfreq  = 20360000;
  
    statPut();
    
    if (SetTimer(hWnd,779, 1000, NULL) == 0) 
      message( hWnd, " Can't Set Timer. ");
  }
  else {
    if (KillTimer(hWnd,779) == 0) 
      message( hWnd, " Can't Kill Timer. ");
  }
  checkfrequency = 0;
  checkpower = 0.0;
}


// ------------------------------------------------------
//
// процедуры для сканирования и изменения таблицы питчей.
//
// ------------------------------------------------------


VOID initScanCount( void ) {
// начальное зануление всех счетчиков (инициализация).

  currenttime = 0; // счетчик времени

  scancirclecount = 0; // число проходов циклического сканирования.

  initproc(&acu);
  initproc(&scan);

  circlefreq = 0;  // циклическое сканирование
  dcirclecount = 0;  

  pvradio = 0;    // Предыдущее значение сигнала в текущей точке
                  // (для ожидания, пока она стабилизируется).
  emptyvradio = TRUE; // признак, пустого pvradio. 

  currentscalepoint.v = 0;
  currentscalepoint.f = 0;

  clearCommStr();

  checkfrequency = 0;
  checkpower = 0.0;
}


VOID checkmx( void ) {
  // отслеживание leftmx, rightmx.
  
  if ( (currentpoint.v > leftmx.v) or
       ( (currentpoint.v == leftmx.v) and (currentpoint.f < leftmx.f) ) )
    leftmx = currentpoint;    
    
  if ( (currentpoint.v > rightmx.v) or
       ( (currentpoint.v == rightmx.v) and (currentpoint.f > rightmx.f) ) )
    leftmx = currentpoint;
}


//***************************************************

void SendCommY( void ) {
// посылка строки в порт - 
//               опрос уровня мощности сигнала в приемнике

  clearCommStr();
  WriteComm( npTTYInfo.idComDev, "y\r\n", 2 ) ;
}


void SendCommSetFreq( long f ) {
// посылка строки в порт - установка частоты приемника

  clearCommStr();
  wsprintf ( commStr, "%ld.%04ld\r\n", f/10000, f%10000 );
  WriteComm( npTTYInfo.idComDev, commStr, strlen(commStr) ) ;
}


//*************************************************** 
// 
//                  А В Т О М А Т
//
//                     ( С К А Н )
//
//***************************************************

static VOID ScanStep( HWND hWnd ) {
// автомат, вызывается по приходу строки от приемника
// и из п/п обработки акустических сообщений

  if ( error )
    return; // на всякий случай
    
  for (;;) {
    switch (scan.state) {
  
// ***********************************************    
// п/п: установка частоты

    case 20: 
      SendCommSetFreq(currentpoint.f);      
      exitstate(scan,30);
      
    case 30:
      if ( commStrPtr != 0 ) {
        if ( tryAgainOnCommError ) {
          // можно еще попробовать повторить предыдущую команду
          // несколько раз          
          SendCommSetFreq(currentpoint.f);      
          exitstate(scan,30);
        }
        else {  
          // или выдать сообщение об ошибке:        
          message( hWnd, " comm err len ");
          error = TRUE;
          exitstate(scan,0);
        }  
      }
      returnstate(scan);

//************************************************    
// п/п: опрос уровня сигнала

    case 40:
      emptyvradio = TRUE;
      
    case 45:    
      SendCommY();
      exitstate(scan,50);
      
    case 50:  
      if ( commStrPtr != 1 ) {
        if ( tryAgainOnCommError ) {
          // можно еще попробовать повторить предыдущую команду
          // несколько раз
          SendCommY();
          exitstate(scan,50);
        }  
        else {
          // или выдать сообщение об ошибке:        
          message( hWnd, " comm err len ");
          error = TRUE;
          exitstate(scan,0);
        }  
      }
      
      if ( commStr[0] == '%' ) 
        currentpoint.v = 0;
      else {
        if ( (commStr[0] < 'A') || (commStr[0] > ('A'+15))) {
          if ( tryAgainOnCommError ) {
            // можно еще попробовать повторить предыдущую команду
            // несколько раз
            SendCommY();
            exitstate(scan,50);
          }
          else {
            // или выдать сообщение об ошибке:        
            message( hWnd, " comm err char ");
            commStr[1] = 0;
            message( hWnd, commStr );
            currentpoint.v = 0;
            error = TRUE;
            exitstate(scan,0);
          }  
        }  
        else {
          currentpoint.v = commStr[0]-'A'+1;
          if (emptyvradio or (currentpoint.v != pvradio)) {
            pvradio = currentpoint.v;
            emptyvradio = FALSE;
            gotostate(scan,45);
          } 
        }    
      }
      returnstate(scan);
      
//************************************************    
// п/п: Установка частоты & опрос сигнала      

    case 70: 
      callstate(scan,20,80); // Установка частоты
      
    case 80:
      callstate(scan,40,90); // Опрос сигнала
      
    case 90:
      returnstate(scan);
      
//************************************************    
// п/п: Scan around currentpoint.

    case 200: 
      
      leftpoint = currentpoint;
      rightpoint = currentpoint;
      
      // squelchMode = 2;
       
      leftmx = currentpoint;
      rightmx = currentpoint;
      
    case 210:  // Scan to the left
	  circlefreqReport();
     
      if (currentpoint.v == 0) {
        currentpoint = rightpoint;
        gotostate(scan,250);
      }  
      else {
        leftpoint = currentpoint;
        if ( leftpoint.f <= pitch.firstfreq+smallstep ) {
          gotostate(scan,250);
        }  
        else {
          leftpoint.f -= smallstep;
          currentpoint = leftpoint;
          callstate(scan,70,220);
        }
      }    
       
    case 220:
      checkmx();
      gotostate(scan,210);
      
    case 250:  // Scan to the right
      circlefreqReport();
      if (currentpoint.v == 0) {
        gotostate(scan,300);
      }  
      else {
        rightpoint = currentpoint;
        if ( rightpoint.f >= pitch.lastfreq-smallstep ) {
          gotostate(scan,300);
        }  
        else {
          rightpoint.f += smallstep;
          currentpoint = rightpoint;
          callstate(scan,70,260);
        }
      }
      
    case 260:
      checkmx();
      gotostate(scan,250);
      
    case 300:    
      currentpoint = rightpoint;
      returnstate(scan);
 
//************************************************    
      // начало сканирования

   case 690:  
      initScanCount();
      acu.state = 1;
      newCircle();         // повтор
      currentpoint.f = pitch.firstfreq;
      currentpoint.v =  0;

//************************************************    

// *******************************************************
// циклическое сканирование
    
    case 2000: 
      circlefreq += bigstep;
      if ( circlefreq >= pitch.lastfreq-pitch.firstfreq ) {
        scancirclecount++;
        if ( scancirclecount >= nofcircles ) {
          message(hWnd,"   End Of Scan   ");
          acu.state = 0;
          SendMessage(hWnd,WM_CLOSE,0,0);
          exitstate(scan,0);
        }
        newCircle();
      }        

      currentpoint.f = circlefreq+pitch.firstfreq;
      callstate(scan,70,2100);
      
    case 2100:
      if ( currentpoint.v != 0 ) {
        callstate(scan,200,2200); // Scan around
      }
      gotostate(scan,2000);
            
    case 2200:  ;
      // при циклическом сканировании обнаружен новый максимум
      if ((rightpoint.f-leftpoint.f) < 2000) {
        // не проверяем 
        gotostate(scan,810);  
      }  

      if ( acu.state != 1 ) {
        errormessage(hWnd, "err acu state");
        exitstate(scan,0);
      }

      // настройка на станцию. 
      currentpoint.f = (leftmx.f+rightmx.f)/2;
      checkfrequency = currentpoint.f;
      callstate(scan,70,800);

//************************************************
// проверка станции (уже настроились).
      
    case 800:
      // запуск акупроца.
      acu.state = 400;
      acuproc(hWnd);
      // ожидание конца проверки станции.
      exitstate(scan,1);
    
    case 810:  
      // это состояние устанавливает acuproc.
      // (конец проверки станции).
      currentpoint.f = rightpoint.f;
      while ( circlefreq <= (currentpoint.f-pitch.firstfreq) )
        circlefreq += bigstep;
      circlefreq -= bigstep;
      gotostate(scan,2000);
     
//************************************************    
// не бывает

    case 0:
    default:
      errormessage(hWnd, "Scan State Error");      
      exitstate(scan,0);
     } 
    }
  }


//**********************************************************
//
//                      A C U   P R O C
//
//**********************************************************


VOID StartWaveIn( HWND hWnd ) {
// запуск оцифровки

  waveAddBuffer(hWnd,&pb);
  waveStart(hWnd);
}


static void acuproc( HWND hWnd ) {

// Обработка буфера с акустическим сигналом
// (акустический процесс).

  for (;;) 
  switch ( acu.state ) {
    case 100: 
      setDirection(hWnd,waveInp);
      scan.state = 690; 
      ScanStep(hWnd);
      exitstate(acu,1);

    case 400:
      contScan = FALSE;
      xmode = FALSE;
    
    case 410:
      
      StartWaveIn(hWnd);
      exitstate(acu,420);

    case 420:
      
      if ( contScan ) {
        contScan = FALSE;
        scan.state = 810;
        ScanStep(hWnd);
        exitstate(acu,1);
      }  
      
      if ( !xmode ) 
        putWave();  

      gotostate(acu,410); 
      
       
    default:
      message(hWnd, "acu proc state error");
      error = TRUE;
      exitstate(acu,0);
      break;
  }
}


BOOL FAR PASCAL __export AR3000DialogProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {
// Test Mode (Send String) Dialog.

  switch (msg) {
    case WM_INITDIALOG:
      clearCommStr();
      return (TRUE);

    case WM_COMMAND:
      if ( wParam == IDCANCEL) {
        EndDialog(hDlg, 2);
        return (TRUE);
      }
	  if (wParam == IDOK) {
        if (GetWindowText( GetDlgItem(hDlg,IDC_COMMAND), (LPSTR)commStr, 15)) {
          strcat(commStr,"\r");
          WriteComm( npTTYInfo.idComDev,commStr,strlen(commStr) ) ;
        }  
	  }
	  break;
  }
  return (FALSE);
}


static long doubleToLong( double f ) {
  return (long)f;
}  


static long getFrequency(HWND hDlg, int ID_CONTROL, char *errstr) {
  double f;
  char *s;

  if (!GetWindowText( GetDlgItem(hDlg,ID_CONTROL), 
                      (LPSTR)commStr, 15)) {
    message( hDlg, errstr ) ;
    return -1;
  }  

  f = strtod(commStr,&s);
            
  if ( ( s[0] != 0 ) and (s[0] != ' ') ) {
    message( hDlg, errstr );
    return -1;
  }
        
  if ( f < 0.0001 ) {
    message( hDlg, errstr );
    return -1;
  }

  if ( f > 2036.0000001 ) {
    message( hDlg, errstr );
    return -1;
  }
  
  return doubleToLong(f*10000.0);
}


static long getCount(HWND hDlg, int ID_CONTROL,char *errstr) {
  double f;
  char *s;

  if (!GetWindowText( GetDlgItem(hDlg,ID_CONTROL), 
                      (LPSTR)commStr, 15)) {
    message( hDlg, errstr ) ;
    return -1;
  }  

  f = strtod(commStr,&s);
            
  if ( ( s[0] != 0 ) and (s[0] != ' ') ) {
    message( hDlg, errstr );
    return -1;
  }
        
  if ( f < 0.9999999 ) {
    message( hDlg, errstr );
    return -1;
  }

  if ( f > 1000.000 ) {
    message( hDlg, errstr );
    return -1;
  }
  
  return doubleToLong(f);
}


BOOL FAR PASCAL __export ScanParamDialogProc(HWND hDlg,
    unsigned msg, WORD wParam, LONG lParam) {
// Get Scan Parameters Dialog.

  long nf1,nf2,ns,nc;

  switch (msg) {
    case WM_INITDIALOG:
      printW( hDlg, IDC_GET_F1,  (LPSTR)"%ld.%ld",
                       pitch.firstfreq/10000, (pitch.firstfreq%10000)/1000 );
      printW( hDlg, IDC_GET_F2,  (LPSTR)"%ld",pitch.lastfreq/10000 );
      printW( hDlg, IDC_NSOUND,  (LPSTR)"%d", nofsounds );
      printW( hDlg, IDC_NCIRCLE, (LPSTR)"%d", nofcircles );
      return (TRUE);

    case WM_COMMAND:
      switch ( wParam ) {

      case IDC_COMMAND:            
        // посылка команд пользователя в приемник
        myDialogBox( hDlg, "IDD_COMMAND", (FARPROC) AR3000DialogProc);
		return TRUE;  
		  
      case IDCANCEL:
        EndDialog(hDlg, 0);
		return (TRUE);

      case IDOK:
        nf1 = getFrequency(hDlg,IDC_GET_F1,"Error in first frequency value");
        nf2 = getFrequency(hDlg,IDC_GET_F2,"Error last frequency value");
        ns = getCount(hDlg,IDC_NSOUND,"Error (number of sounds) value");
        nc = getCount(hDlg,IDC_NCIRCLE,"Error (number of loops) value");
        
        if ( (nf1 < 0) or
             (nf2 < 0) or
             (ns < 0) or
             (nc < 0) )
          return TRUE;   

        // инит:
        
        pitch.firstfreq = nf1;
        pitch.lastfreq = nf2;
        
        nofsounds = (short)ns;
        nofcircles = (short)nc;
        
		statPut();
        EndDialog(hDlg, 1);
        return (TRUE);
      }
      break;
    }
    return (FALSE);
}

// Рисования 

VOID ClearScale(HDC rDC) {
  HBRUSH  hOldBrush;
  RECT rF;
  short i;
  
  if ( error or (fwScale.hWnd == NULL) )
    return;

  rF = fwScale.rfs;
  hOldBrush = SelectObject(rDC, pen.bGray);
  Rectangle(rDC,rF.left,rF.top,rF.right,rF.bottom);
  SelectObject(rDC, hOldBrush);
  
  if (pitch.radio != NULL) 
    loop (i,pitch.mscale) 
      pitch.radio[i].pv = 0;
}


static VOID putHScale(HDC rDC, RECT *rF, char fAmp) {
// рисование шкалы сканирования.
  HPEN    hOldPen;
  short   i,x0,y0,x1,dy,xM,yM;
  double  fs;
  short dxy = 3;
  scalepoint far *s;
  
  if ( error or (fwScale.hWnd == NULL)  or ( pitch.radio == NULL ) )
    return;    
    
  s = pitch.radio;
  x0 = rF->left+dxy;
  y0 = rF->top+dxy;
  x1 = rF->right-dxy;
  dy = (rF->bottom-rF->top)-dxy*2;
  
  if ( ( pitch.mscale < 1 ) or
       ( pitch.lastfreq < 1 ) or 
       ( x1 <= x0 ) or 
       ( pitch.lastfreq < pitch.firstfreq ) or
       ( dy < 1 ) ) {
    ClearScale(rDC);
    return;
  }  
    
  fs = (x1-x0) / (double)(pitch.mscale);
  hOldPen = SelectObject( rDC, pen.hGrayPen );
    
  loop (i, pitch.mscale)
    if ( s[i].v != s[i].pv ) { 
      xM = x0+round(fs*i);    

      SelectObject ( rDC, pen.hGrayPen );
      MoveTo (rDC,xM,y0-1);
      LineTo (rDC,xM,y0+dy+1);
    
      SelectObject( rDC, GetStockObject (WHITE_PEN) );
      MoveTo (rDC,xM,y0+dy);
      if ( s[i].v > fAmp )
        s[i].v = fAmp;
      if ( s[i].v < 0 )
        s[i].v = 0;  
      yM = y0+dy-round( (((double)(s[i].v))*dy)/fAmp );
      LineTo (rDC,xM,yM);
      s[i].pv = s[i].v;
    }  
  SelectObject(rDC, hOldPen);
}


static VOID putScale(HDC rDC) {
  putHScale(rDC, &(fwScale.rfs), 17 );
  statReport(rDC);
}


static VOID paintScale( void ) {
  HDC rDC;
  
  WINDOWPLACEMENT wndpl; 

  if ( error or ( fwScale.hWnd == NULL ) )
    return;

  if ( GetWindowPlacement(fwScale.hWnd,&wndpl) ) {
    if ( (wndpl.showCmd & SW_SHOWMINIMIZED) != 0 )
      return;
  }

  rDC = GetDC( fwScale.hWnd );
  putScale( rDC );
  statReport(rDC);
  ReleaseDC(fwScale.hWnd, rDC);
}    


static VOID getScaleRect ( void ) {
  RECT 		rW;
  UINT	    drX = 3,
            drY = 3;
    
  if ( error or (fwScale.hWnd == NULL) )
    return;

  GetClientRect(fwScale.hWnd, &rW);
    
  fwScale.rf.left = drX;    
  fwScale.rf.right = rW.right-drX;
    
  fwScale.rf.top = drY;
  fwScale.rf.bottom = rW.bottom-drY;  
  
  fwScale.rfs.left = drX;

  fwScale.rfs.right = rW.right-drX;
  
  fwScale.rfs.top = drY;
  fwScale.rfs.bottom = (rW.bottom-drY);
}


static VOID resizeScaleWin( HWND hWnd, HDC hdc ) {
  RECT 		rW;
  HBRUSH 	hBrushW, hOldBrush;
  HPEN   	hPenW, hOldPen;
    
  if ( error or (fwScale.hWnd == NULL) )
    return;

  getScaleRect();
  
  hPenW = GetStockObject(WHITE_PEN);
  hBrushW = GetStockObject(WHITE_BRUSH);

  GetClientRect(hWnd, &rW);
    
  hOldPen = SelectObject(hdc, hPenW);
  hOldBrush = SelectObject(hdc, hBrushW);
    
  Rectangle(hdc, rW.left, rW.top, rW.right, rW.bottom);
    
  SelectObject(hdc, hOldPen);
  SelectObject(hdc, hOldBrush);
}


//**********************************************************
//
//    *   *   ***  ***  *  *      *   *  ***  *  *
//    ** **  *  *   *   ** *      *   *   *   ** *
//    * * *  ****   *   * **      * * *   *   * **
//    *   *  *  *  ***  *  *       * *   ***  *  *
//
//**********************************************************

LONG FAR PASCAL __export ScaleWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  
    switch (msg)
    {	    
    case WM_COMMAND:
      switch (wParam) {
	    case IDCANCEL :
          acu.state = 0;
          scan.state = 0;
          SendMessage(hWnd,WM_CLOSE,0,0);
          break;

	    default:
	      return DefWindowProc(hWnd,msg,wParam,lParam);         
      }	  
      break;
        
    case WM_COMMNOTIFY:
      if (ProcessCOMMNotification(hWnd,(WORD)wParam,(LONG)lParam) == TRUE) {
        if (scan.state != 0)
          ScanStep(hWnd);
        else
          clearCommStr();
      }
      break;

    case WM_CHAR:
       if ( LOBYTE( wParam ) == ' ' ) 
         contScan = TRUE;
       if ( LOBYTE( wParam ) == 'x' ) 
         if ( xmode )
           xmode = FALSE;
         else  
           xmode = TRUE;
       break ;

    case MM_WIM_DATA: // End Of Record.
      waveDone(hWnd,lParam);
      copyWave(&pb);
      if ( acu.state > 1)
        acuproc(hWnd);
      break;
      
    case WM_TIMER:
      currenttime++;
      if ( scan.state > 5 )
		paintScale();
      break;
    
    case WM_CREATE:
      ScanOn(hWnd,TRUE);
      break;
      
    case WM_CLOSE:
      DestroyWindow(hWnd);
      break;  
      
    case WM_DESTROY:
      ScanOn(hWnd,FALSE);
      ColorsOn(FALSE);
	  PostQuitMessage(0);
	  break;

	case WM_PAINT:
      hdc = BeginPaint(hWnd,&ps);
      ClearScale(hdc);
      if ( fwScale.hWnd != NULL )      
        putScale(hdc);
      EndPaint(hWnd,&ps);
	  break;

    case WM_SIZE: 
      hdc = GetDC( hWnd );
      resizeScaleWin( hWnd, hdc );
      ClearScale(hdc);
      if ( fwScale.hWnd != NULL )
        putScale( hdc );
      ReleaseDC( hWnd,hdc );
      break;

    default:
      return DefWindowProc(hWnd,msg,wParam,lParam);         
    }

    return NULL;
}


VOID PASCAL MakeScaleWin(HWND hWndO, short x0, short y0, short dx, short dy)
{
    WNDCLASS    wc;
    UINT	    drX = 10,
                drY = 10;
    LPCSTR		fClassName = "ScaWinCls";
    LPCSTR      fwName = "Frequency Scale";            
    
    /* Define and register a window class for the main window.
     */
    
  if ( error or ( fwScale.hWnd != NULL ) )
    return;
      
  if ( !ScaleOn ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION);
        wc.lpszMenuName   = (LPSTR)NULL; 
        wc.lpszClassName  = fClassName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wc.hInstance      = ghInst;
        wc.style          = 0;
        wc.lpfnWndProc    = ScaleWindowProc;
        wc.cbWndExtra     = 0;
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        ScaleOn = TRUE;  
    }

  fwScale.hWnd = CreateWindow (fClassName,  // class name
                            fwName,        	// caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			x0,y0,dx,dy,
                            (HWND)hWndO,    // parent window
                            (HMENU)NULL,    // use class menu
                            (HANDLE)ghInst, // instance handle
                            (LPSTR)NULL     // no params to pass on
                           );

  getScaleRect();

  ShowWindow(fwScale.hWnd, SW_SHOWNORMAL);   
}


BOOL InitInstance(HANDLE hInstance, int nCmdShow) {
// порождение окон

  HWND hWnd;

  ghInst = hInstance;
  ColorsOn(TRUE);
  fwScale.hWnd = NULL;

  MakeScaleWin(NULL, 0, 0,
  	GetSystemMetrics(SM_CXSCREEN),
  	GetSystemMetrics(SM_CYSCREEN)/2);

  hWnd = fwScale.hWnd;

  if (!hWnd)
    return (FALSE);

  makeWaveWin(hWnd,	0, 
    GetSystemMetrics(SM_CYSCREEN)/2,
  	GetSystemMetrics(SM_CXSCREEN),
  	GetSystemMetrics(SM_CYSCREEN)/2);

  initproc(&acu);
  initproc(&scan);
  
  if ( myDialogBox( hWnd, "IDD_START_OPTIONS", (FARPROC) ScanParamDialogProc) != 1 ) {
    SendMessage(hWnd,WM_CLOSE,0,0);
    return FALSE;
  }  
    
  // Start

  scan.state = 0;
  acu.state = 100;
  acuproc(hWnd);
  return (TRUE);
}


int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) 
{
  MSG msg;
  
  if (hPrevInstance)
    return FALSE;
    
  if (!InitInstance(hInstance, nCmdShow))
    return (FALSE);

  while (GetMessage(&msg, NULL, NULL, NULL)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (msg.wParam);
}

