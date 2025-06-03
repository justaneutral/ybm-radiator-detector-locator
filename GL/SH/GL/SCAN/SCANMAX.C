//________________________________________________
//                                                           
//                   ScanMax.c                            
//________________________________________________

#include "cinc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fcomm.h"
#include "port.h"
#include "resource.h"


typedef char vint;  // значение радио-мощности (0..16).
typedef long fint;  // частота
typedef struct { fint f; vint v; } fpoint; // частота, мощность
typedef struct { char v, pv, live; } scalepoint; // текущее / нарисованное значения

typedef struct { fint fl, fr; } freqdiap; // диапазон частот

static freqdiap scand; // диапазон сканируемых частот

static short mscale; // размер масивов для рисования    

static scalepoint FAR *radio; // рисуемые мощности радиостанций


// акустическая проверка

#define msample 5000
#define mcheck  3000 // проверяемый кусок

static DOUBLEARRAY  silence;

static double      // для проверок радиостанций:
     lcheck = 0.0,
     rcheck = 0.0, // средние амплитуды сигнала до и после стукоа
     dlcheck = 0.0, // дисперсии
     lpower = 0.0,
     drcheck = 0.0;
     
static short nNeg = 0; // число неправильных проверок, у которых s1 > s2.
                         // для принятия решения что не жук.

// счетчики, статистика

static short nofcircles = 5; // число проходов по частотам
static short nofsounds = 1;  // минимальное число стуков на радиостанцию
static short checkbuzzloop = 0;  // число стуков

static long scancirclecount = 0; // число проходов циклического сканирования
static fint checkfrequency = 0;  // для отчета об акустической проверке.
static double checkpower = 0.0;

static waveBuffer pb;
static BOOL wavemode = FALSE;


// Автомат сканирования

static fint circlefreq;
static short dcirclecount = 0;  // счетчик для отслеживания начального смещения
                                // частот в циклическом сканировании

static freqdiap // текущий сигнал
  signal,  // границы сигнала
  smax;    // границы максимума сигнала
  
static vint vsmax; // максимальное значение

  static vint  pvradio = 0; // Предыдущее значение сигнала в текущей точке
                       // (для ожидания, пока она стабилизируется)
  static BOOL  emptyvradio = TRUE; // признак, пустого pvradio
  static fint bigstep = 5000, // шаг сканирования при отсутствии сигнала
       smallstep  =  500; // шаг сканирования в присутствии сигнала
  static BOOL tryAgainOnCommError = FALSE; // реакция на ошибку Comm

static fpoint currentpoint; // для передачи параметров (текущая точка сканирования)

//int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int); // main win

//static BOOL InitInstance(HANDLE, int);

static sprocess acu,scan;

static void acuproc( HWND );

// **************************************************************

static BOOL ScaleOn = FALSE; // признак того, что оконный класс уже зарегестрирован.

static struct  {
   HWND   hWnd;  
   RECT   rf,   // общий прямоугольник
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
    sc[i].v = currentpoint.v;
    sc[i].live = 5;
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
  "From  %ld.%04ld mHz  (%ld.%04ld mHz)  to  %ld.%04ld mHz. nChecks %ld.          ",
    (scand.fl/10000),
    (scand.fl%10000)/1000,
    (tf/10000),
    (tf%10000)/1000,
    (scand.fr/10000),
    (scand.fr%10000)/1000,
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


//
//****************** On / Off ***********************
//

static VOID ScanOn( HWND hWnd, BOOL St ) {
// инициализация для акустической части программы
  short j;

  
  doubleOn(hWnd, &sl, msample, St);
  doubleOn(hWnd, &sr, msample, St);
  
  doubleOn(hWnd, &silence, msample, St);
  
  waveOn(hWnd,2,2,1,St); // hWnd, nOfBytes,nOfChannels,frequency,St

  bufferOn(hWnd, &pb, msample*4L, 0, St);
  
  // инициализация или освобождение памяти для шкалы частот

  if (St)
    mscale = GetSystemMetrics(SM_CXSCREEN);
  
  memOn(hWnd, &(radio), mscale*sizeof(scalepoint), St);

  ConnectionOn( hWnd, St );

  if ( error ) 
    return;

  if ( St ) {
    outport(0);

    loop (j,mscale) {
      radio[j].v  = 0;
      radio[j].pv = 0;
      radio[j].live = 0;
    }  
    
    getMemDC(radio)->m = mscale;
    scancirclecount = 0; // число проходов циклического сканирования.
    wavemode = FALSE;

    initproc(&acu);
    initproc(&scan);     

    circlefreq = 0;
    dcirclecount = 0;

    pvradio = 0; // Предыдущее значение сигнала в текущей точке
                    // (для ожидания, пока она стабилизируется).
    emptyvradio = TRUE; // признак пустого pvradio. 
    
    scand.fl =     1000;
    scand.fr  = 20360000;
  
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


  scancirclecount = 0; // число проходов циклического сканирования.

  wavemode = FALSE;

  initproc(&acu);
  initproc(&scan);

  circlefreq = 0;  // циклическое сканирование
  dcirclecount = 0;  

  pvradio = 0;    // Предыдущее значение сигнала в текущей точке
                  // (для ожидания, пока она стабилизируется).
  emptyvradio = TRUE; // признак, пустого pvradio. 

  clearCommStr();

  checkfrequency = 0;
  checkpower = 0.0;
}


VOID newCircle( void ) {
// инициализация по пачалу нового прохода циклического сканирования
  static short dcircle[5] = {0,3000,1000,4000,2000};
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
  
  loop (i, mscale) {
    if ( radio[i].live > 0 ) {
      radio[i].live--;
      
      if ( radio[i].live < 1) {
        radio[i].v = 0;
      }  
    }
  }  
}


VOID checkmx( void ) {
  // отслеживание smax.fl, smax.fr.
  
  if ( (currentpoint.v > vsmax) or
       ( (currentpoint.v == vsmax) and (currentpoint.f < smax.fl) ) ) {
    smax.fl = currentpoint.f;    
    vsmax = currentpoint.v;
  }
    
  if ( (currentpoint.v > vsmax) or
       ( (currentpoint.v == vsmax) and (currentpoint.f > smax.fr) ) ) {
    smax.fl = currentpoint.f;    
    vsmax = currentpoint.v;
  }  
}


//***************************************************

void SendCommY( void ) {
  // посылка строки в порт - 
  //   опрос уровня мощности сигнала в приемнике

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
          errormessage( hWnd, " comm err len ");
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
      
      signal.fl = signal.fr = currentpoint.f;
      smax.fr = smax.fl = currentpoint.f;
      vsmax = currentpoint.v;
      
    case 210:  // Scan to the left
	  putScalePoint( radio, currentpoint.f, currentpoint.v );
     
      if (currentpoint.v == 0) {
        currentpoint.f = signal.fr;
        gotostate(scan,250);
      }  
      else {
        signal.fl = currentpoint.f;
        if ( signal.fl <= scand.fl+smallstep ) {
          gotostate(scan,250);
        }  
        else {
          signal.fl -= smallstep;
          currentpoint.f = signal.fl;
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
        signal.fr = currentpoint.f;
        if ( signal.fr >= scand.fr-smallstep ) {
          gotostate(scan,300);
        }  
        else {
          signal.fr += smallstep;
          currentpoint.f = signal.fr;
          callstate(scan,70,260);
        }
      }
      
    case 260:
      checkmx();
      gotostate(scan,250);
      
    case 300:    
      currentpoint.f = signal.fr;
      returnstate(scan);
 
//************************************************    
// начало сканирования

   case 690:  
      initScanCount();
      acu.state = 1;
      newCircle();         // повтор
      currentpoint.f = scand.fl;
      currentpoint.v =  0;

//************************************************
// циклическое сканирование
    
    case 2000: 
      circlefreq += bigstep;
      if ( circlefreq >= scand.fr-scand.fl ) {
        scancirclecount++;
        if ( scancirclecount >= nofcircles ) {
          message(hWnd,"   End Of Scan   ");
          acu.state = 0;
          SendMessage(hWnd,WM_CLOSE,0,0);
          exitstate(scan,0);
        }
        newCircle();
      }        

      currentpoint.f = circlefreq+scand.fl;
      callstate(scan,70,2100);
      
    case 2100:
      if ( currentpoint.v != 0 ) {
        callstate(scan,200,2200); // Scan around
      }
      gotostate(scan,2000);
            
    case 2200:  ;
      // при циклическом сканировании обнаружен новый максимум
      if ((signal.fr-signal.fl) < 2000) {
        // не проверяем 
        gotostate(scan,810);  
      }  

      if ( acu.state != 1 ) {
        errormessage(hWnd, "err acu state");
        exitstate(scan,0);
      }

      // настройка на станцию. 
      currentpoint.f = (smax.fl+smax.fr)/2;
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
      currentpoint.f = signal.fr;
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


static VOID sort(DOUBLEARRAY a, short l, short r) {
// sort a[l..r];

  short i,j;
  double x,y;
  
  i = l;
  j = r;
  x = a[ (l+r) / 2 ];
  
  do {
    while ( a[i] < x )
      i++;
  
    while ( x < a[j] )
      j--;
      
    if ( i <= j ) {
      y = a[i]; 
      a[i] = a[j]; 
      a[j] = y;
      i = i+1; 
      j = j-1;
    }
  } while ( i <= j );
  
  if ( l < j )
    sort(a,l,j);
  if ( i < r )
    sort(a,i,r);
}


double getPower1( void ) {
// оценка уровня сигнала (не используется).

  double s;
  int i;
  
  s = 0.0;
  
  vtrend(sr);
  
  vLowFilter(sr);
  loop(i,mcheck) 
    s += fabs(sr[i+150]);
  
  return s;  
}


double getPower( void ) {
// оценка уровня сигнала с медианным сглаживанием

  vtrend(sr);
  vabs(sr,sr);
  sort(sr,200,200+mcheck);
  return sr[200+mcheck/2];
}


short getCheckPower( HWND hWnd, short tn, double s1, double s2 ) {
// 
// обработка оцифрованного массива со стуком,
//  возвращает:
//
//    1 - конец проверки, не жук,
//    2 - продолжить проверку,
//    3 - конец проверки, жук.
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


static void acuproc( HWND hWnd ) {

// Обработка буфера с акустическим сигналом
// (акустический процесс).

  short i;
  double tpower;
  static BOOL checkOrder = TRUE;     

  for (;;) 
  switch ( acu.state ) {
    case 100:  //  начало работы 
      setDirection(hWnd,waveInp);
      scan.state = 690; 
      ScanStep(hWnd);
      exitstate(acu,1);

    case 400: // начало стуков
    
      checkOrder = TRUE;
      checkbuzzloop = -1;
      StartWaveIn(hWnd);
      outport(6);      
      exitstate(acu,420);

    case 420:
      
      // первый стук
      StartWaveIn(hWnd);
      if ( checkOrder )
        outport(6);
      else  
        outport(5);
      exitstate(acu,440);
    
    case 440:
      lpower = getPower();
      vequ(silence,sr);
      
      // второй стук
      StartWaveIn(hWnd);
      if ( checkOrder )
        outport(5);
      else  
        outport(6);
      exitstate(acu,460);

    case 460:  

      checkbuzzloop++;
      tpower = getPower();
      
      if ( checkOrder )
        i = getCheckPower ( hWnd, checkbuzzloop, lpower, tpower );
      else  
        i = getCheckPower ( hWnd, checkbuzzloop, tpower, lpower );
        
      statPut();

      if ( checkOrder ) {
        vequ(sl,sr); // для сиимметрии
        vequ(sl,silence);
      }  
      else {
        vequ(sl,sr);
        vequ(sr,silence);
      }  
      
      putWave();  

      if ( checkbuzzloop <= 1 ) {
        lcheck = 0.0;
        rcheck = 0.0;
        dlcheck = 0.0;
        drcheck = 0.0;
        nNeg = -2;
      }  
      
      switch (i) {
        case 0:
          errormessage(hWnd,"intern scan err");
          exitstate(acu,0);
          
        case 1:  
          scan.state = 810;
          ScanStep(hWnd);
          exitstate(acu,1);
          
        case 2:  
          checkOrder = (rand() > (RAND_MAX/2));
          gotostate(acu,420);
          
        case 3:  
          gotostate(acu,500);
        }
        break;
        
    case 500: // нашли
        loop(i,10)
          MessageBeep((UINT)-1);
          
        if ( MessageBox( hWnd, "Radio transceiver is found.",
               "Locate ?", MB_YESNO|MB_ICONQUESTION) == IDYES)  {
                 
          setDirection(hWnd,waveNull);
          WinExec( "c:\\sh\\gl\\gant\\gant.exe", SW_SHOW );
          scan.state = 0;
          SendMessage(hWnd,WM_CLOSE,0,0);
          exitstate(acu,0);          
        }
        else {
          scan.state = 810;
          ScanStep(hWnd);
          exitstate(acu,1);
        }               
      
    default:
      errormessage(hWnd, "acu proc state error");
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
                       scand.fl/10000, (scand.fl%10000)/1000 );
      printW( hDlg, IDC_GET_F2,  (LPSTR)"%ld",scand.fr/10000 );
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
        
        scand.fl = nf1;
        scand.fr = nf2;
        
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
  
  if (radio != NULL) {
    loop (i,mscale) {
      //radio[i].v = 0;
      radio[i].pv = 0;
    }  
  }    
    
}


static VOID putHScale(HDC rDC, RECT *rF, char fAmp) {
// рисование шкалы сканирования.
  HPEN    hOldPen;
  short   i,x0,y0,x1,dy,xM,yM;
  double  fs;
  short dxy = 3;
  scalepoint FAR *s;
  
  if ( error or (fwScale.hWnd == NULL)  or ( radio == NULL ) )
    return;    
    
  s = radio;
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
    
  fs = (x1-x0) / (double)mscale;
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

    case MM_WIM_DATA: // End Of Record.
      wavemode = FALSE;
      waveDone(hWnd,lParam);
      copyWave(&pb);
      if ( acu.state > 1)
        acuproc(hWnd);
      break;
      
    case WM_TIMER:
      if ( (wavemode == FALSE) and ( scan.state > 5 ) )
		paintScale();
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
  	GetSystemMetrics(SM_CXSCREEN),
  	GetSystemMetrics(SM_CYSCREEN)/2);

  initproc(&acu);
  initproc(&scan);
  
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

