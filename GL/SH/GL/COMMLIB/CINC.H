#include <windows.h>
#include <mmsystem.h>


// ***********************************************
//               ����  mem.c
// ***********************************************

extern HINSTANCE ghInst;

// ���������� ����������� �������.
typedef struct GlobalArrayTag {
  HGLOBAL h;
  long l; // ������ ������� � ������ (��� ���������).
  short m; // ��� ��� ( ����� ��������� ��� double � �.� ). 
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
// ���������� ���������� ����������� �������.

BOOL doubleOn (HWND, DOUBLEARRAY *, short, BOOL );
// ���������� ������� double � ���������� ������.

// --- ������ / ���������� �������� ---

BOOL readMem (HWND hWnd, HFILE hfFile, VOID *ga );
// ������ �� ����� � ������������ ���������� ������

BOOL openMem (HWND hWnd, HFILE hfFile, VOID **ga );
// ������ �� ����� � ����������� ���������� �������

BOOL saveMem (HWND hWnd, HFILE hfFile, VOID *ga );
// ������ ����������� ������� � ����
 

// **********************************************
//                  ���� AdcDac 
// **********************************************

typedef struct oneSampleTag { // ��� ������ 16.
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
      *next,   // ������ ���� �������
      *nextOn; // ������ ������� � ������� � ��������         
  } waveBuffer, *lpWaveBuffer;                   

VOID  waveOn ( HWND, short, short, short, BOOL );
VOID  bufferOn ( HWND, lpWaveBuffer, long, short, BOOL );
VOID  setDirection( HWND, waveDirection );
VOID  waveAddBuffer ( HWND, lpWaveBuffer );
VOID  waveStart ( HWND );
short waveDone ( HWND, LONG );


// ***********************************************
/* ������� ������ � midi-�������. ���� (midi.c) */
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
//                     ���� Paint.c 
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
//                    ���� waveWin.c
// ****************************************************
//
//                     (���� "Line In")

extern DOUBLEARRAY sl,sr;
VOID  copyWave ( waveBuffer * );

VOID makeWaveWin(HWND, short, short, short, short);
VOID putWave( void );
// BOOL waveOn ( void );

// _____________ ���� GraWin.c ( ������ ������� ) ____________

typedef struct {
   HWND         hWnd; 
   LPCSTR       szName; // caption
   DOUBLEARRAY  d;
   RECT         RF;
} FUNWIN, *LPFUNWIN;

VOID makeGraWin(HWND, LPFUNWIN, short, short, short, short);

VOID putGra(FUNWIN *);

// ______________________________________________________
//   ��������� ��� ��������� �������� �������� � �����.

#define WSound (331.0/11025.0)


// ******************************************
//                 ���� mmath.c
// ******************************************


#define     loop(i,j)        for ( i=0; i<j; i++ )
#define     loop2(i,j1,j2)   for ( i = j1; i < j2; i++ )

#define 	not !
#define 	and &&
#define 	or ||

#define		Pi 3.1415926


// ���. ��������� ������ ����������.

// ������� ����� ���������

int sign( double );
double sqr( double );
double fmax( double, double );
double fmin( double, double );

//   �������� � double ���������

short absP(DOUBLEARRAY);
short minP(DOUBLEARRAY);
short maxP(DOUBLEARRAY);

VOID vtrend(DOUBLEARRAY); 
VOID vnormir(DOUBLEARRAY); // ���������� ������� (abs(f)<=1).

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
void	fftAmp( void );      /* ���������� �������� */
void	vplane( double [] ); /* mid, log */
void	logAmp( void );      // ���������������� ��������
BOOL    fftOn ( HWND, BOOL ); // Init

// -----------------------------------------
//                   LPC
// -----------------------------------------

#define     ma        17
#define     map       18  // (ma+1)

void 	initXemm( DOUBLEARRAY d ); // initFft �������� initXemm.
void 	corr( DOUBLEARRAY, DOUBLEARRAY );
int 	autoc( double *, double * );

// ����������

VOID vStreakFilter( DOUBLEARRAY ); // ��������� ������
VOID vLowFilter( DOUBLEARRAY );    // �������������� ������


//************************************************    
//    ���� mprocess.c
//************************************************    

//    ����� ��� ��������� ��������� ��������

#define mstatestack 10 // ������� ����� ���������

typedef struct {
    short state;  // ��������� ��������
    short statestack[mstatestack]; // ���� ���������
    short stackptr;
  } sprocess; 
  
#define gotostate(p,st)      { p.state = st; break; }
#define exitstate(p,st)      { p.state = st; return; }
#define callstate(p,st1,st2) { callstateproc(&p,st1,st2); break; }
#define returnstate(p)       { retstateproc(&p); break; }


//************************************************

VOID initproc( sprocess * );

VOID callstateproc( sprocess *, short gotoStateNom, short retState );
// ��������� ��������� �������� � ����������� ��������� ��� 
// �������� � ���� ��������� �������� (�������� ������ ���������).

VOID retstateproc( sprocess * );


// **************************************************************
//   ���� outwin.c

HWND  createOutWin( HWND hWndO, int nCmdShow, short, short, short, short );
int   outprint(LPCSTR fmt, ...);

extern HWND outWin;