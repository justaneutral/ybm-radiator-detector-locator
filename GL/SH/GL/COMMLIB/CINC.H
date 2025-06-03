#include <windows.h>
#include <mmsystem.h>


// ***********************************************
//               Файл  mem.c
// ***********************************************

extern HINSTANCE ghInst;

// дескриптор глобального массива.
typedef struct GlobalArrayTag {
  HGLOBAL h;
  long l; // размер массива в байтах (без заголовка).
  short m; // доп инф ( число элементов для double и т.д ). 
} GLOBALARRAY, *LPGLOBALARRAY;

typedef double *DOUBLEARRAY;
extern BOOL error;

VOID errorstring( char * );
BOOL checkError ( HWND hWnd );
int message(HWND hDlg, char *fmt, ...);
// message      ( HWND, LPCSTR );
VOID errormessage ( HWND, LPCSTR );
int  myDialogBox  ( HWND, LPCSTR, FARPROC );
BOOL memOn ( HWND, VOID **, long, BOOL );

LPGLOBALARRAY getMemDC ( VOID * );
// возвращает дескриптор глобального массива.

BOOL doubleOn (HWND, DOUBLEARRAY *, short, BOOL );
// размещение массива double в глобальной памяти.

// --- чтение / сохранение массивов ---

BOOL readMem (HWND hWnd, HFILE hfFile, VOID *ga );
// чтение из файла в существующий глобальный массив

BOOL openMem (HWND hWnd, HFILE hfFile, VOID **ga );
// чтение из файла в создаваемый глобальный массива

BOOL saveMem (HWND hWnd, HFILE hfFile, VOID *ga );
// запись глобального массива в файл
 

// **********************************************
//                  Файл AdcDac 
// **********************************************

typedef struct oneSampleTag { // для стерео 16.
   int leftSample,
       rightSample;
  } oneSample;

typedef enum { 
  waveNull,
  waveInp,
  waveOut
} waveDirection;
  
typedef enum waveBufferStateTag {
    bfUnPrep,
    bfPrepInp,
    bfPrepOut,
    bfInp,
    bfOut
  } waveBufferState;

typedef struct waveBufferTag {
    WAVEHDR WaveHdr;
    HANDLE hWaveData;
    waveBufferState state;
    waveDirection device;
    struct waveBufferTag 
      *next,   // список всех буферов
      *nextOn; // список буферов в очереди к драйверу         
  } waveBuffer, *lpWaveBuffer;                   

VOID  waveOn ( HWND, short, short, short, BOOL );
VOID  bufferOn ( HWND, lpWaveBuffer, long, short, BOOL );
VOID  setDirection( HWND, waveDirection );
VOID  waveAddBuffer ( HWND, lpWaveBuffer );
VOID  waveStart ( HWND );
short waveDone ( HWND, LONG );


// ***********************************************
/* Утилиты работы с midi-файлами. Файл (midi.c) */
// ***********************************************

VOID SendString(HWND, LPCSTR);
BOOL midiOn(HWND, LPCSTR, LPCSTR, BOOL);
VOID midiRePlay(HWND, LPCSTR);
VOID midiReset(HWND, LPCSTR);
VOID midiPlay(HWND, LPCSTR);
VOID midiStop(HWND, LPCSTR);
short midiStatus(HWND, LPCSTR);
  /* 1 - plaing, 3 - stopped. */


// **************************************************
//                     Файл Paint.c 
// **************************************************

extern struct penTag {
    HPEN pGreen,hDashPen,hDotPen,pHGreen,hGrayPen;
    HBRUSH bGray,bGreen;
  } pen;

short round( double f );
BOOL ColorsOn( BOOL ); /* INIT DIALOG. */
VOID drawAmp(HDC, RECT *, DOUBLEARRAY);
VOID drawF(HDC, RECT *, DOUBLEARRAY);
int printW(HWND, UINT, char *, ...);

//
// ****************************************************
//                    Файл waveWin.c
// ****************************************************
//
//                     (Окно "Line In")

extern DOUBLEARRAY sl,sr;
VOID  copyWave ( waveBuffer * );

VOID makeWaveWin(HWND, short, short, short, short);
VOID putWave( void );
// BOOL waveOn ( void );

// _____________ файл GraWin.c ( График функции ) ____________

typedef struct {
   HWND         hWnd; 
   LPCSTR       szName; // caption
   DOUBLEARRAY  d;
   RECT         RF;
} FUNWIN, *LPFUNWIN;

VOID makeGraWin(HWND, LPFUNWIN, short, short, short, short);

VOID putGra(FUNWIN *);

// ______________________________________________________
//   множитель для упрощения перевода отсчетов в метры.

#define WSound (331.0/11025.0)


// ******************************************
//                 Файл mmath.c
// ******************************************


#define     loop(i,j)        for ( i=0; i<j; i++ )
#define     loop2(i,j1,j2)   for ( i = j1; i < j2; i++ )

#define 	not !
#define 	and &&
#define 	or ||

#define		Pi 3.1415926


// Мат. процедуры общего назначения.

// простые общие процедуры

int sign( double );
double sqr( double );
double fmax( double, double );
double fmin( double, double );

//   операции с double массивами

short absP(DOUBLEARRAY);
short minP(DOUBLEARRAY);
short maxP(DOUBLEARRAY);

VOID vtrend(DOUBLEARRAY); 
VOID vnormir(DOUBLEARRAY); // Нормировка функции (abs(f)<=1).

VOID Corrff( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID Corrbff( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID vmul( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID vdiv( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID vadd( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID vsub( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID vequC( DOUBLEARRAY, double );
VOID vequ( DOUBLEARRAY, DOUBLEARRAY );
VOID vabs( DOUBLEARRAY, DOUBLEARRAY );

// -----------------------------------------
//                  F F T 
// -----------------------------------------

#define pFft	8
#define mFft	256	/* Size Of Data Block = (2**pFft) */ 
#define mFftH   128	/* mFft/2 */

#define pi2	6.283185303
#define pi	3.1415926

typedef	double	fftArr[mFft];
typedef double  fftArrH[mFftH];

extern	DOUBLEARRAY  fftX,fftY,fftXemm;

void	fft( void );		 /* fft	*/
void	fftm( void );		 /* back */
void	fftAmp( void );      /* вычисление амплитуд */
void	vplane( double [] ); /* mid, log */
void	logAmp( void );      // логарифмирование амплитуд
BOOL    fftOn ( HWND, BOOL ); // Init

// -----------------------------------------
//                   LPC
// -----------------------------------------

#define     ma        17
#define     map       18  // (ma+1)

void 	initXemm( DOUBLEARRAY d ); // initFft вызывает initXemm.
void 	corr( DOUBLEARRAY, DOUBLEARRAY );
int 	autoc( double *, double * );

// фильтрация

VOID vStreakFilter( DOUBLEARRAY ); // выделение питчей
VOID vLowFilter( DOUBLEARRAY );    // низкочастотный фильтр


//************************************************    
//    файл mprocess.c
//************************************************    

//    макро для изменения состояния автомата

#define mstatestack 10 // глубина стека переходов

typedef struct {
    short state;  // состояние автомата
    short statestack[mstatestack]; // стек переходов
    short stackptr;
  } sprocess; 
  
#define gotostate(p,st)      { p.state = st; break; }
#define exitstate(p,st)      { p.state = st; return; }
#define callstate(p,st1,st2) { callstateproc(&p,st1,st2); break; }
#define returnstate(p)       { retstateproc(&p); break; }


//************************************************

VOID initproc( sprocess * );

VOID callstateproc( sprocess *, short gotoStateNom, short retState );
// изменение состояния автомата и запоминание состояния для 
// возврата в стек состояний автомата (имитация вызова процедуры).

VOID retstateproc( sprocess * );


// **************************************************************
//   файл outwin.c

HWND  createOutWin( HWND hWndO, int nCmdShow, short, short, short, short );
int   outprint(LPCSTR fmt, ...);

extern HWND outWin;