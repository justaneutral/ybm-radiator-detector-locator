#include "cinc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fcomm.h"
#include "resource.h"

// акустическая проверка
typedef char vint;  // значение радио-мощности (0..16).
typedef long fint;  // частота
typedef struct { fint f; vint v; } fpoint; // частота, мощность
typedef struct { char v, pv, live; } scalepoint; // текущее / нарисованное значения

typedef struct { fint fl, fr; } freqdiap; // диапазон частот

static freqdiap scand; // диапазон сканируемых частот
static short mscale; // размер масивов для рисования    
static scalepoint FAR *radio; // рисуемые мощности радиостанций
static scalepoint FAR *after; // рисуемые мощности радиостанций
static scalepoint FAR *before; // рисуемые мощности радиостанций

// акустическая проверка

#define msample   mFft
static long scancirclecount = 0; // число проходов циклического сканирования
static waveBuffer pb;

static BOOL wavemode = FALSE; // ожидается прерывание от Sound Blaster'a.
  
static double soundMode = -1.0; // признак, если  меньше нуля, то 
                                  // шумно и можно проверять
  
#define msignal  100 // максимальное число радиостанций
#define mchdiap    3 // число диапазонов проверки
#define macucount 10 // число циклов для измерения уровня шума

typedef struct { double cl0, cr0, crl, clm, crm; } CRL;

typedef struct {
    double p;        // приоритет
    fint fs,fl,fm;   // диапазон частот
    char live;       // счетчик для удаления
    CRL cr[mchdiap]; // для вычисления корреляции
    int np;          // число проверок;
    double wp;       // сумма результатов проверок
} SIGNAL, FAR *LPSIGNAL;

static LPSIGNAL signal; // таблица радиостанций

long checkRelation = 0; // соотношение числа акустических проверок
                           // и сканирования
static fint checkfrequency = 0; // для отчетности

int 
  checkSignal,     // текущий проверяемый сигнал
  nsignal;         // текущее число радоистанций

// Автомат сканирования

static fint circlefreq;

static short dcirclecount = 0;  // счетчик для отслеживания начального смещения
                                // частот в циклическом сканировании
                                
static short dcircle[5] = {0,3000,1000,4000,2000};

static freqdiap 
           sigdiap,
           smax;
static vint vsmax;           

fpoint currentpoint; // для передачи параметров (текущая точка сканирования)

sprocess acu,scan;

static void acuproc( HWND );

// **************************************************************

static BOOL ScaleOn = FALSE; // признак того, что оконный класс уже зарегестрирован.

#define mscalewin 3

static struct  {
   HWND          hWnd;  
   RECT           rf,  // общий прямоугольник
                  rfs[mscalewin];  // шкала сканирования
}  fwScale = {NULL};

static FUNWIN FFTX = {NULL,"FFTX",NULL,{0,0,0,0}};
static FUNWIN FFTY = {NULL,"FFTY",NULL,{0,0,0,0}};
 
// **************************************************************
// п/п рисования. 

short ftos( fint f ) {
// вычисления места в шкале сканирования, соответсвщего частоте f.
  fint fdiap,fpos;
  short i,m;
  
  if ( error )
    return 0;
    
  fdiap = scand.fr-scand.fl+1;
  if ( fdiap < 1 )
    return 0;
  fpos = currentpoint.f-scand.fl;
  if ( ( fpos < 0 ) or ( fpos >= fdiap ) )
    return 0;  
  m = mscale;
  i = round((((double)fpos)*m)/(double)fdiap);
  if ( i >= m )
    i = m-1;   
  if ( i < 0 )
    i = 0;
  return i;  
}


VOID putScalePoint( scalepoint *sc, fint f, vint v ) {
// изменение шкалы сканирования
// currentpoint содержит текущую частоту и мощность.

  long i;
  
  if ( error )
    return;
    
  i = ftos(f);
  
  if ( ( sc[i].live != 5 ) or ( sc[i].v < v ) ) {
    sc[i].live = 5;
    sc[i].v = v;
  }  
}


static VOID statReport(HDC hDC) {
// выдача сообщений о текущей частоте в окно состояния программы
// (перерисовка окна со статистикой).

  TEXTMETRIC textmetric;
  int nDrawX;
  int nDrawY;
  char szText[300];
  
  fint tf;
  
  tf = circlefreq+scand.fl;
  
  if ( tf < scand.fl )
    tf = scand.fl;
  if ( tf > scand.fr )
    tf = scand.fr;  
    
  sprintf(szText,
  "From  %ld.%01ld mHz  (%ld.%01ld mHz)  to  %ld.%01ld mHz. nChecks %ld. %d         ",
    (scand.fl/10000),
    (scand.fl%10000)/1000,
    (tf/10000),
    (tf%10000)/1000,
    (scand.fr/10000),
    (scand.fr%10000)/1000,
    scancirclecount, scan.state );

  GetTextMetrics (hDC, &textmetric);

  nDrawX = GetDeviceCaps (hDC, LOGPIXELSX) / 4;   /* 1/4 inch */
  nDrawY = GetDeviceCaps (hDC, LOGPIXELSY) / 4;   /* 1/4 inch */
  
  //SetBkColor(hDC,RGB(128,128,128));
   SetTextColor( hDC, RGB(0,0,0));//FGCOLOR( npOutInfo ) ) ;
   SetBkColor( hDC, RGB(128,128,128)); //GetSysColor( COLOR_WINDOW ) ) ;

  TextOut (hDC, nDrawX, nDrawY, szText, strlen (szText));
  
  if ( fwScale.hWnd != NULL )
    SendMessage(fwScale.hWnd,WM_SETTEXT,0,(LPARAM)(LPCSTR)szText);
    
  nDrawY += textmetric.tmExternalLeading + textmetric.tmHeight;

  sprintf(szText,
  "Check %ld.%03ld MHz.             ",
    (checkfrequency/10000),
    (checkfrequency%10000)/10);
  TextOut (hDC, nDrawX, nDrawY, szText, strlen (szText));
}


VOID statPut( void ) {
// перерисовка окна со статистикой
  HDC hDC;
  
  hDC = GetDC (fwScale.hWnd);
  statReport(hDC);
  ReleaseDC(fwScale.hWnd,hDC);
}


//
//****************** On / Off ***********************
//

static VOID ScanOn( HWND hWnd, BOOL St ) {
// инициализация для акустической части программы
  short j;

  doubleOn(hWnd, &sl, msample, St);
  doubleOn(hWnd, &sr, msample, St);
  
  waveOn(hWnd,2,2,1,St); // hWnd, nOfBytes,nOfChannels,frequency,St
  
  fftOn(hWnd,St);
  
  FFTX.d = fftX;
  FFTY.d = fftY;

  bufferOn(hWnd, &pb, msample*4L, 0, St);
  
  // инициализация или освобождение памяти для шкалы частот

  if (St)
    mscale = GetSystemMetrics(SM_CXSCREEN);
  
  memOn(hWnd, &radio,  mscale*sizeof (scalepoint), St);
  memOn(hWnd, &after,  mscale*sizeof (scalepoint), St);
  memOn(hWnd, &before, mscale*sizeof (scalepoint), St);

  memOn(hWnd, &signal, msignal*sizeof(SIGNAL), St);
  
  ConnectionOn( hWnd, St );

  if ( error ) 
    return;

  if ( St ) {
    loop (j,mscale) {
      radio[j].v  = 0;
      radio[j].pv = 0;
      radio[j].live = 0;
      
      after[j].v  = 0;
      after[j].pv = 0;
      after[j].live = 0;
      
      before[j].v  = 0;
      before[j].pv = 0;
      before[j].live = 0;
    }  
    
    getMemDC(radio)->m = mscale;
    getMemDC(after)->m = mscale;
    getMemDC(before)->m = mscale;

    scancirclecount = 0; // число проходов циклического сканирования.
    checkRelation = 0;

    wavemode = FALSE;

    initproc(&acu);
    initproc(&scan);     

    circlefreq = 0;
    dcirclecount = 0;

//    scand.fl =     1000;
//    scand.fr  = 20360000;
    scand.fl =  4000000;
    scand.fr  = 5000000;
  
    statPut();
    
    if (SetTimer(hWnd,779, 5000, NULL) == 0) 
      message( hWnd, " Can't Set Timer. ");
  }
  else {
    if (KillTimer(hWnd,779) == 0) 
      message( hWnd, " Can't Kill Timer. ");
  }
  checkfrequency = 0;
}


// ------------------------------------------------------
//
// процедуры для сканирования и изменения таблицы питчей.
//
// ------------------------------------------------------


VOID initScanCount( void ) {
// начальное зануление всех счетчиков (инициализация).

  scancirclecount = 0; // число проходов циклического сканирования.

  wavemode = FALSE;

  initproc(&acu);
  initproc(&scan);

  circlefreq = 0;  // циклическое сканирование
  dcirclecount = 0;  

  clearCommStr();

  checkfrequency = 0;
}


VOID checkmx( void ) {
  // отслеживание smax.
  
  if ( (currentpoint.v > vsmax) or
       ( (currentpoint.v == vsmax) and (currentpoint.f < smax.fl) ) ) {
    smax.fl = currentpoint.f;    
    vsmax = currentpoint.v;
  }  
    
  if ( (currentpoint.v > vsmax) or
       ( (currentpoint.v == vsmax) and (currentpoint.f > smax.fr) ) ) {
    smax.fr = currentpoint.f;
    vsmax = currentpoint.v;
  }  
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


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

static int getSignalNom( fint fs, fint fl ) {
  int i,im;
  fint dmin,td;
  LPSIGNAL s;

  if ( error )
    return -1;
  
  im = -1;
  dmin = scand.fr*2;
  
  loop ( i, nsignal ) {
    s = signal+i;
    if ( ( s->fs <= fl ) and ( s->fl >= fs ) ) {
      td = labs( (s->fs+s->fl)-(fs+fl) );
      if ( td < dmin ) {
        dmin = td;
        im = i;
      }
    }    
  }
  return im;
}


static BOOL newSignalNom( fint fs, fint fl, fint fm, char sv ) {
  LPSIGNAL s;

  if ( error )
    return FALSE;
    
  if ( nsignal >= msignal )
    return FALSE;
    
  s = signal+nsignal;
  
  s->fs = fs;
  s->fl = fl;
  s->fm = fm;
  s->p = (fl-fs)*sv;
  s->live = 5;
  s->np = 0;
  s->wp = 0.0;
  nsignal++;
}


static BOOL repSignalNom( fint fs, fint fl, fint fm, char sv, int nst ) {
// обновление станции
  LPSIGNAL s;

  if ( error )
    return FALSE;
    
  if ( nsignal >= msignal )
    return FALSE;
    
  s = signal+nst;

  s->fs = fs;
  s->fl = fl;
  s->fm = fm;
  s->p = (fl-fs)*sv;
  s->live = 5;
}


static VOID delSignal( int nst ) {
  int i;
  
  if ( (nsignal < 1) or (nst >= nsignal) ) {
    error = TRUE;
    return;
  }
    
  loop2 (i, nst+1, nsignal ) 
    signal[i-1] = signal[i];

  nsignal--;
}


static int getCheckSignal ( void ) {
  int i;
  double p, r;

  if ( nsignal < 1 ) 
    return -1;
    
  if ( checkRelation < 0 )
    return -1;  
    
  if ( nsignal == 1 )
    return 0;  

  p = 0.0;
    
  loop ( i, nsignal )
    p += signal[i].p;
    
  r = ( rand() * p *1.1 ) / RAND_MAX;
  
  i = 0;
  for (;;) {
    r -= signal[i].p;
    if ( r < 0.0 )
      return i;
    i++;
    if ( i >= nsignal )
      i = 0;
  }
}


VOID newCircle( void ) {
// инициализация по пачалу нового прохода циклического сканирования
  short i;

  if ( error )
    return;
    
  dcirclecount++;
  if ( dcirclecount > 4 )
    dcirclecount = 0;
  circlefreq = dcircle[dcirclecount];  
  
  loop (i, mscale) {
    if ( radio[i].live > 0 ) {
      radio[i].live--;
      
      if ( radio[i].live < 1) {
        radio[i].v = 0;
        after[i].v = 0;
        before[i].v = 0;
      }  
    }
  }  

  i = 0;
  
  while ( i < nsignal ) {
    if ( signal[i].live <= 0 )
      delSignal(i);
    else {
      signal[i].live--;
      i++;  
    }
  }  
}


VOID circleMax( void ) {
// при циклическом сканировании обнаружен сигнал
  fint fm;
  int i;
  
  fm = (smax.fl+smax.fr)/2;
  
  i = vsmax;
  
  outprint("(%ld.%03ld MHz)=%d, S=%4.0f\r",fm/10000,(fm%10000)/10,i,soundMode/10);
    
  if ((sigdiap.fr-sigdiap.fl) < 2000) 
    return; // не учитываем 
  
  i = getSignalNom ( sigdiap.fl, sigdiap.fr );
  
  if ( i < 0 )
    newSignalNom ( sigdiap.fl, sigdiap.fr, fm, vsmax );
  else
    repSignalNom ( sigdiap.fl, sigdiap.fr, fm, vsmax, i );  
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

static vint pvradio = 0; // Предыдущее значение сигнала в текущей точке
                       // (для ожидания, пока она стабилизируется)
                       
static BOOL emptyvradio = TRUE; // признак, пустого pvradio

static fint bigstep = 5000, // шаг сканирования при отсутствии сигнала
       smallstep =  250; // шаг сканирования в присутствии сигнала

static BOOL tryAgainOnCommError = FALSE; // реакция на ошибку Comm

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
          errormessage( hWnd, " comm err len ");
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
            errormessage( hWnd, commStr );
            currentpoint.v = 0;
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
      
      sigdiap.fl = currentpoint.f;
      sigdiap.fr = currentpoint.f;
      
      // squelchMode = 2;
       
      smax.fl = currentpoint.f;
      smax.fr = currentpoint.f;
      
    case 210:  // Scan to the left
	  putScalePoint( radio, currentpoint.f, currentpoint.v );
     
      if (currentpoint.v == 0) {
        currentpoint.f = sigdiap.fr;
        gotostate(scan,250);
      }  
      else {
        sigdiap.fl = currentpoint.f;
        if ( sigdiap.fl <= scand.fl+smallstep ) {
          gotostate(scan,250);
        }  
        else {
          sigdiap.fl -= smallstep;
          currentpoint.f = sigdiap.fl;
          callstate(scan,70,220);
        }
      }    
       
    case 220:
      checkmx();
      gotostate(scan,210);
      
    case 250:  // Scan to the right
      putScalePoint( radio, currentpoint.f, currentpoint.v );
      if (currentpoint.v == 0) {
        gotostate(scan,300);
      }  
      else {
        sigdiap.fr = currentpoint.f;
        if ( sigdiap.fr >= scand.fr-smallstep ) {
          gotostate(scan,300);
        }  
        else {
          sigdiap.fr += smallstep;
          currentpoint.f = sigdiap.fr;
          callstate(scan,70,260);
        }
      }
      
    case 260:
      checkmx();
      gotostate(scan,250);
      
    case 300:    
      currentpoint.f = sigdiap.fr;
      returnstate(scan);
 
//************************************************    
      // начало сканирования

   case 690:  
      initScanCount();
      acu.state = 1;
      newCircle();         // повтор
      currentpoint.f = scand.fl;
      currentpoint.v =  0;

// *******************************************************
// циклическое сканирование
    
    case 2000: 
      if ( soundMode > 0.0) {
        checkSignal = getCheckSignal();
        if ( checkSignal >= 0 )
          gotostate(scan,2500);
      }
      checkRelation++;
      circlefreq += bigstep;
      if ( circlefreq >= scand.fr-scand.fl ) {
        scancirclecount++;
        newCircle();
      }        

      currentpoint.f = circlefreq+scand.fl;
      callstate(scan,70,2100);
      
    case 2100:
      if ( currentpoint.v != 0 ) {
        callstate(scan,200,2200); // Scan around
      }
      gotostate(scan,2000);
            
    case 2200:
      // при циклическом сканировании обнаружен новый максимум
      circleMax();
      gotostate(scan,2000);

      // настройка на станцию. 
    case 2500:
      currentpoint.f = signal[checkSignal].fm; 
      checkfrequency = currentpoint.f;
      callstate(scan,70,800);

// ************************************************
//    проверка станции ( уже настроились ).
      
    case 800:
      // запуск акупроца.
      checkRelation -= 10;
      if ( acu.state != 200 ) {
        errormessage(hWnd, "err acu state");
        exitstate(scan,0);
      }

      outprint("<< Check Signal %ld.%03ld MHz >>\r",
         checkfrequency/10000, 
        (checkfrequency%10000)/10 );

      acu.state = 300;
      //acuproc(hWnd);
      // ожидание конца проверки станции.
      exitstate(scan,3);
    
    case 810:  
      // это состояние устанавливает acuproc.
      // (конец проверки станции).
      currentpoint.f = sigdiap.fr;
      while ( circlefreq <= (currentpoint.f-scand.fl) )
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

  wavemode = TRUE;
  waveAddBuffer(hWnd,&pb);
  waveStart(hWnd);
}


/*
short getCheckPower( HWND hWnd, short tn, double s1, double s2 ) {
// 
// обработка оцифрованного массива со стуком,
//  возвращает:
//
//    1 - конец проверки, не жук,
//    2 - продолжить проверку,
//    3 - переход на локацию (принято решение, что жук).
//
//

  double sm, st, rp, lp, rdsp, ldsp, dsp;
  
  if ( s1 < 1.0 ) s1 = 1.0;
  if ( s2 < 1.0 ) s2 = 1.0;
  
  if (tn < 1) {
    if ( s1 > s2 )
      return 1;
    else
      return 2;  
  }

  if (s1 > s2)
    nNeg++;
    
  lp = log(s1);
  rp = log(s2);
  
  lcheck += lp; 
  rcheck += rp;
  
  dlcheck += (lp*lp);
  drcheck += (rp*rp);

  st = rcheck/tn;
  rdsp = sqrt(drcheck/tn-st*st);
  sm = lcheck/tn;
  ldsp = sqrt(dlcheck/tn-sm*sm);
  dsp = 2.5*(rdsp+ldsp)/sqrt(tn);
  st -= sm;
  checkpower = st;
  
// принятие решения жук / не жук.

  if (tn < 1) {
    error = TRUE;
    return 1;
  }

  if (tn < nofsounds)
    return 2;
  
  if ( ( st < log(1.05) ) or ( nNeg > tn/4 ) )
    return 1;  
    
  if ( tn < 25 )
    return 2;
    
  if ( st < log(1.2) )
    return 1;  

  if ( st > log(1.2)+dsp )
    return 3;
    
  if ( tn < 50 )
    return 2;

  if ( st > log(1.2) )
    return 3;

  return 1;                  
}  
*/

double currentNoise = 0.0;

static VOID noiselevel( void ) {
// вычисление текущего уровня шума
  double s,sp,sn;
  int i;

  sp = sr[0];
  s = 0.0;
  
  loop (i,msample) {
    sn = sr[i];
    sr[i] = sn-sp;
    s += fabs(sr[i]);
    sp = sn;
  }
  currentNoise = s/100.0;
}

/*
typedef struct { double cl0, cr0, crl; } CRL;

typedef struct {
    double p;        // приоритет
    fint fs,fl,fm;      // диапазон частот
    char live;       // счетчик для удаления
    CRL cr[mchdiap]; // для вычисления корреляции
    int np;          // число проверок;
    double wp;       // сумма результатов проверок
} SIGNAL, FAR *LPSIGNAL;

*/

VOID subdiap ( LPSIGNAL s, int diap, int hl, int hr ) {
  double a1,a2;
  int i;

  a1 = 0;
  a2 = 0;

  loop2( i, hl, hr ) {
    a1 += fftX[i];
    a2 += fftY[i];
  }  

  s->cr[diap].crm += a1;
  s->cr[diap].clm += a2;
  s->cr[diap].cr0 += a1*a1;
  s->cr[diap].cl0 += a2*a2;
  s->cr[diap].crl += a1*a2;
}


static double diapcorrel(double rs, double ls, double rl, double rm, double lm) {
  // преобразование сумм в корреляцию
  double upch,loch;
  int nt;
  
  nt = macucount;
  upch = rl-rm*lm/nt;
  loch = sqrt((ls-lm*lm/nt)*(rs-rm*rm/nt));
  if ( loch < 0.01 )
    loch = 0.01;
  return upch/loch;  
}


static double compareSignals( HWND hWnd, int stage ) {
// оценка схожести двух сигналов
  LPSIGNAL s;
  int i,ts;
  double sc;
          
  ts = checkSignal;
  s = signal+ts;  

  switch ( stage ) {
  
    case 1: // начало цикла
      loop (i, mchdiap) {
        s->cr[i].crm = 0.0;
        s->cr[i].clm = 0.0;
        s->cr[i].cr0 = 0.0;
        s->cr[i].cl0 = 0.0;
        s->cr[i].crl = 0.0;
      }
      break;
      
    case 2: // обработка очередного сигнала
      vmul(fftX,sr,fftXemm);
      vmul(fftY,sl,fftXemm);
      
      fft();
      fftAmp();
      logAmp();

      subdiap(s,0,0,30);
      subdiap(s,1,31,70);
      subdiap(s,2,71,mFftH);

      break;
      
    case 3: // завершение цикла
      s->np++;
      sc = 0.0;
      loop (i, mchdiap) 
        sc += diapcorrel(
                     s->cr[i].cr0, 
                     s->cr[i].cl0, 
                     s->cr[i].crl, 
                     s->cr[i].crm, 
                     s->cr[i].clm );
                     
      sc /= mchdiap;
      s->wp = ( (s->wp)*((s->np)-1) + sc ) / (s->np);
      sc = s->wp;      
      sc = fmax(-1.0,sc);
      sc = fmin(1.0, sc)+1.0;
      putScalePoint(after, s->fm, (char)(sc*50.0));
      break;     
  }
  return 0;
}


VOID clearSignal( int ts ) {
// инициализация i-ой радиостанции
  int i;
  LPSIGNAL s;
  
  if ( error )
    return;
    
  s = signal+ts;  
    
  s->p = 0.0;
  s->fs = scand.fl;
  s->fl = scand.fl;
  s->live = 0;
  loop ( i, mchdiap ) {
    s->cr[i].crm = 0.0;
    s->cr[i].clm = 0.0;
    s->cr[i].cr0 = 0.0;
    s->cr[i].cl0 = 0.0;
    s->cr[i].crl = 0.0;
  }
  s->np = 0;
  s->wp = 0.0;
}  

static VOID putW( void ) {
  extern HWND hWaveWnd;

  char szText[300];
  
  sprintf(szText,
  "Check %ld.%03ld MHz:             ",
    (checkfrequency/10000),
    (checkfrequency%10000)/10);

  if ( hWaveWnd != NULL ) {
    SendMessage(hWaveWnd, WM_SETTEXT, 0, (LPARAM)(LPCSTR)szText);
    putWave();
  }  
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
////////////////////////// ACU PROC //////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


static int acucount; // счетчик для измерения уровня шума

static double noise = 10000.0;

static void acuproc( HWND hWnd ) {
// (акустический процесс).

  for (;;) 
  switch ( acu.state ) {
    case 100:  //  начало работы 
      // измерение уровня шума
      vequ(fftX,fftXemm);
      vequ(fftY,fftXemm);
      putGra(&FFTX);
      putGra(&FFTY);

      outprint("\rNoise level...");
      acucount = 0;
      noise = 0.0;
      setDirection(hWnd,waveInp);
      
    case 110:  
      StartWaveIn(hWnd);
      exitstate(acu,120);
      
    case 120:  
      noise += currentNoise;
      acucount++;
      
      if ( acucount < macucount )
        gotostate(acu,110);
        
      noise *= (1.2/macucount);
      outprint(" %8.0f \rStart scan... \r",noise/10);
      
    // вход в цикл измерения уровня шума и запуск сканирования
    
      soundMode = -1.0;
      StartWaveIn ( hWnd );
      
      acu.state = 200;
        
      scan.state = 690; 
      ScanStep ( hWnd );
      exitstate ( acu, 200 );

    case 200:
      soundMode = currentNoise - noise;
      //if ( scan.state == 3 ) // настройка на радиостанцию
      //  exitstate ( acu, 3 );
      StartWaveIn (hWnd);
      exitstate (acu, 200);
        
      
    case 300: // проверка радиостанции ( уже настроились )
      acucount = 0;
      compareSignals ( hWnd, 1 );
      
    case 310:  
      putW();
      StartWaveIn (hWnd);
      exitstate (acu, 320);

    case 315:  
      StartWaveIn (hWnd);
      exitstate (acu, 320);
    
    case 320:
      soundMode = ( currentNoise - noise );
      compareSignals ( hWnd, 2 );
      acucount++;
      if ( acucount < 5 )
        gotostate(acu,315);
      
      // конец цикла проверки 
      
      putW();
      putGra(&FFTX);
      putGra(&FFTY);
      compareSignals ( hWnd, 3 );
      scan.state = 810;

      outprint("Scan...%ld\r",checkRelation);
      StartWaveIn(hWnd);
      ScanStep(hWnd); // запуск сканирования 
                              // (или проверки другой станции).
      exitstate(acu,200); // запуск цикла измерения уровня шума

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

  long nf1,nf2;

  switch (msg) {
    case WM_INITDIALOG:
      printW( hDlg, IDC_GET_F1,  (LPSTR)"%ld.%ld",
                       scand.fl/10000, (scand.fl%10000)/1000 );
      printW( hDlg, IDC_GET_F2,  (LPSTR)"%ld",scand.fr/10000 );
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
        
        if ( (nf1 < 0) or
             (nf2 < 0) )
          return TRUE;   

        // инит:
        
        scand.fl = nf1;
        scand.fr = nf2;
        
        //nofcircles = (short)nc;
        
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

  hOldBrush = SelectObject(rDC, pen.bGray);
  loop (i,mscalewin) {
    rF = fwScale.rfs[i];
    Rectangle(rDC,rF.left,rF.top,rF.right,rF.bottom);
  }  
  SelectObject(rDC, hOldBrush);
  
  if (radio != NULL) 
    loop (i,mscale) {
      radio[i].pv = 0;
    }  
  if (after != NULL) 
    loop (i,mscale) {
      after[i].pv = 0;
    }  
  if (before != NULL) 
    loop (i,mscale) {
      before[i].pv = 0;
    }  
}


static VOID putHScale(HDC rDC, scalepoint FAR *s, RECT *rF, char fAmp) {
// рисование шкалы сканирования.
  HPEN    hOldPen;
  short   i,x0,y0,x1,dy,xM,yM;
  double  fs;
  short dxy = 3;
  //scalepoint far *s;
  
  if ( error or (fwScale.hWnd == NULL)  or ( s == NULL ) )
    return;    
    
  //s = radio;
  x0 = rF->left+dxy;
  y0 = rF->top+dxy;
  x1 = rF->right-dxy;
  dy = (rF->bottom-rF->top)-dxy*2;
  
  if ( ( mscale < 1 ) or
       ( scand.fr < 1 ) or 
       ( x1 <= x0 ) or 
       ( scand.fr < scand.fl ) or
       ( dy < 1 ) ) {
    ClearScale(rDC);
    return;
  }  
    
  fs = (x1-x0) / (double)(mscale);
  hOldPen = SelectObject( rDC, pen.hGrayPen );
    
  loop (i, mscale)
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
  putHScale(rDC, radio, &(fwScale.rfs[0]), 17 );
  putHScale(rDC, before, &(fwScale.rfs[1]), 100 );
  putHScale(rDC, after, &(fwScale.rfs[2]), 100 );
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
            drY = 3,
            i,hr;
    
  if ( error or (fwScale.hWnd == NULL) )
    return;

  GetClientRect(fwScale.hWnd, &rW);
    
  fwScale.rf.left = drX;    
  fwScale.rf.right = rW.right-drX;
    
  fwScale.rf.top = drY;
  fwScale.rf.bottom = rW.bottom-drY;  
  
  hr = (rW.bottom-drY*(mscalewin+1))/mscalewin+drY;
  
  loop (i,mscalewin) {
    fwScale.rfs[i].left = drX;

    fwScale.rfs[i].right = rW.right-drX;
  
    fwScale.rfs[i].top = drY+hr*i;
    fwScale.rfs[i].bottom = drY+hr*(i+1)-drY;
  }  
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
  //static putWaveMode = FALSE;
  
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
        if (scan.state > 5)
          ScanStep(hWnd);
        else
          clearCommStr();
      }
      break;

    case MM_WIM_DATA: // End Of Record.
      wavemode = FALSE;
      waveDone(hWnd,lParam);
      copyWave(&pb);
      noiselevel();
      if ( acu.state > 1)
        acuproc(hWnd);
      break;
      
    case WM_TIMER:
      paintScale();
      //putWaveMode = TRUE;
      //if ( (wavemode == FALSE) and ( scan.state > 5 ) )
		
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
  //InitWaveWindow( );

  MakeScaleWin(NULL, 0, 0,
  	GetSystemMetrics(SM_CXSCREEN),
  	GetSystemMetrics(SM_CYSCREEN)/2);

  hWnd = fwScale.hWnd;

  if (!hWnd)
    return (FALSE);

  ScanOn(hWnd, TRUE);
  
  makeWaveWin(hWnd,	0, 
    GetSystemMetrics(SM_CYSCREEN)/2,
  	GetSystemMetrics(SM_CXSCREEN)/2,
  	GetSystemMetrics(SM_CYSCREEN)/2);

  initproc(&acu);
  initproc(&scan);
  
  createOutWin(hWnd,nCmdShow,
    GetSystemMetrics(SM_CXSCREEN)/2,
    GetSystemMetrics(SM_CYSCREEN)/2,
  	GetSystemMetrics(SM_CXSCREEN)/2,
  	GetSystemMetrics(SM_CYSCREEN)/2
  );
  
  makeGraWin ( hWnd, &FFTX,  0,
              GetSystemMetrics(SM_CYSCREEN)/2,
  	          GetSystemMetrics(SM_CXSCREEN)/4,	
  	          GetSystemMetrics(SM_CYSCREEN)/4);
  	          
  makeGraWin ( hWnd, &FFTY,  
  	          GetSystemMetrics(SM_CXSCREEN)/2,
              GetSystemMetrics(SM_CYSCREEN)/2,
  	          GetSystemMetrics(SM_CXSCREEN)/4,	
  	          GetSystemMetrics(SM_CYSCREEN)/4);

  if ( myDialogBox( hWnd, "IDD_START_OPTIONS", (FARPROC) ScanParamDialogProc) != 1 )
    return FALSE;
    
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

