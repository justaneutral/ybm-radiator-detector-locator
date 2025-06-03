#include <windows.h>
#include <mmsystem.h>
#include "resource.h"
/* ________________________________________________ */


// ***********************************************
//               Файл Mem.c
// ***********************************************

typedef struct GlobalArrayTag {
  HANDLE h;
  short m; // число элементов ( для double и т.д ). 
} GLOBALARRAY;

typedef GLOBALARRAY FAR *LPGLOBALARRAY;
typedef double FAR *DOUBLEARRAY;
extern BOOL CommonError;

VOID Message(HWND Window, LPCSTR s);
int  myDialogBox( HWND hWnd, LPCSTR lpszTemplate, FARPROC lpDlgProc);

BOOL MemoryOn(VOID FAR **po, long l, BOOL St);
LPGLOBALARRAY GetMemDC( VOID FAR *p );
// возвращает дескриптор глобального массива.
// BOOL FreeMem( VOID FAR *p );  // Global Free.
BOOL DoubleOn(HWND hWnd, short m, DOUBLEARRAY *da, BOOL St);
// размещение массива double в глобальной памяти.

 
// **********************************************
//                  Файл AdcDac 
// **********************************************
// _____________________________________________


typedef struct OneSampleTag {
   int leftSample,
       rightSample;
  } OneSample;

typedef enum WaveBufferStateTag {
    bfNull,
    bfIdle,
    bfPrepInp,
    bfPrepOut
  } WaveBufferState;

typedef struct WaveBufferTag {
    WAVEHDR WaveHdr;
    HANDLE hWaveData;
    WaveBufferState state;
    struct WaveBufferTag FAR *next;          
  } WaveBuffer;                   

typedef WaveBuffer FAR *lpWaveBuffer;

VOID CopyWave( WaveBuffer *pb );

BOOL  checkOk(HWND Window, short ErrCode, BOOL inpDev);
VOID  FillWaveFormat(PCMWAVEFORMAT *, short);
VOID  prepareInp(HWND Window, lpWaveBuffer bf);
VOID  prepareOut(HWND Window, lpWaveBuffer bf);
VOID  unprepareOut(HWND Window, lpWaveBuffer bf);
VOID  unprepareInp(HWND Window, lpWaveBuffer bf);
VOID  WaveOut(HWND Window, lpWaveBuffer bf);
VOID  WaveInp(HWND Window, lpWaveBuffer bf);
VOID  OpenDevice(HWND Window, BOOL inpDev);
VOID  CloseDevice(HWND Window, BOOL inpDev);
VOID  OpenBuffer(HWND Window, long dataSize,
                  short bfCode, lpWaveBuffer bf);
BOOL  WaveOn(HWND Window, BOOL St, short);

//extern BOOL WaveError;
extern HWAVEIN hWaveInp;
extern HWAVEOUT hWaveOut;
extern lpWaveBuffer WaveBufferList;



// ***********************************************
/* Утилиты работы с midi-файлами. Файл (midi.c) */
// ***********************************************



VOID SendString(HWND Window, LPCSTR s);

BOOL midiOn(HWND Window, LPCSTR filename, LPCSTR channel, BOOL St);

VOID midiRePlay(HWND Window, LPCSTR channel);

VOID midiReset(HWND Window, LPCSTR channel);

VOID midiPlay(HWND Window, LPCSTR channel);

VOID midiStop(HWND Window, LPCSTR channel);

short midiStatus(HWND Window, LPCSTR channel);
  /* 1 - plaing, 3 - stopped. */


// **************************************************
//                     Файл Paint.c 
// **************************************************

extern struct penTag {
    HPEN pGreen,hDashPen,hDotPen,pHGreen,hGrayPen;
    HBRUSH bGray,bGreen;
  } pen;

  
short round( double f );

BOOL ColorsOn( BOOL St ); /* INIT DIALOG. */

VOID drawAmp(HDC rdc, RECT *rF, DOUBLEARRAY d);

VOID drawF(HDC rDC, RECT *rF, DOUBLEARRAY d);

int printW(HWND hDlg, UINT ne, char *fmt, ...);


//
// ****************************************************
//                    Файл PaintWav.c
// ****************************************************
//
// ____________________________________________________
//
//  процедуры, относящиеся к окнам "Line in" и Fun Win.
// ____________________________________________________
//

typedef struct  FunWindowTag {
   LPCSTR       szName; // application name
   HWND         hWnd;  // window name
   RECT         RF;
//   double       max,min;
   BOOL         On;
   DOUBLEARRAY  d;
} FUNWIN;

extern FUNWIN waveWin;

typedef FUNWIN FAR *LPFUNWIN;

extern DOUBLEARRAY sl,sr;

extern HINSTANCE ghInst;

// ____________________ Wave Win ____________________
//                     (Окно "Line In")
VOID InitWaveWindow( void );

VOID PASCAL MakeWaveWin( HWND );

LONG FAR PASCAL __export WaveWindowProc( HWND, UINT, WPARAM, LPARAM );

// __________________ Gra Win __________________
//                 График функции 

VOID InitGraWin( LPFUNWIN, DOUBLEARRAY, LPCSTR );

VOID PASCAL MakeGraWin( HWND, LPFUNWIN, short );

LONG FAR PASCAL __export GraWindowProc( HWND, UINT, WPARAM, LPARAM );


// __________________ Dir Win __________________
//	             Окно с направлением

VOID InitDirWin( LPFUNWIN, LPCSTR );

VOID PASCAL MakeDirWin(HWND hWndO, LPFUNWIN lpFW);

LONG FAR PASCAL __export DirWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

//  простые общие процедуры

int sign( double x );
double sqr( double x );
double fmax( double f1, double f2 );
double fmin( double f1, double f2 );

//   операции с double массивами

short absD( DOUBLEARRAY );
short minD( DOUBLEARRAY );
short maxD( DOUBLEARRAY );

VOID trend( DOUBLEARRAY ); 
VOID normirFun( DOUBLEARRAY ); // Нормировка функции (abs(f)<=1).

VOID Corrff( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID Corrbff( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID arrMul( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID arrDiv( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID arrAdd( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID arrSub( DOUBLEARRAY, DOUBLEARRAY, DOUBLEARRAY );
VOID arrEquC( DOUBLEARRAY, double );
VOID arrEqu( DOUBLEARRAY, DOUBLEARRAY );
VOID arrAbs( DOUBLEARRAY, DOUBLEARRAY );

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

// -----------------------------------------
//                   LPC
// -----------------------------------------

#define     ma        10
#define     map       11  // (ma+1)

void 	initXemm( DOUBLEARRAY ); // initFft вызывает initXemm.
void 	corr( DOUBLEARRAY, DOUBLEARRAY );
int 	autoc( double [], DOUBLEARRAY  );

// fft

void	fft( void );		/* fft	*/
void	fftm( void );		/* back	*/
void	fftAmp( void );     /* abs  */
void	planeArr( double [] );  /* mid, log */
void	logAmp( void );
BOOL    FftOn ( HWND, BOOL ); // Init

// фильтрация

VOID streakFilter( DOUBLEARRAY ); 
VOID lowFilter( DOUBLEARRAY );    // низкочастотный фильтр

