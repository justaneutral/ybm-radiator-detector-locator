
               /* Wave Format */

#define 	NSAMPLE_IN		1700
#define 	N_CHANNEL		2     
#define 	N_SAMPLE_BYTE	2                  
#define 	N_FREQ			11025L

#define		NSAMPLE_OUT     4

         /* Wave Buffer State (io) : */

#define     IO_PREP_IN    	1 
#define     IO_PREP_OUT   	2 
#define     IO_IN         	3 
#define     IO_OUT        	4 
#define     IO_EMPTY      	5  
#define     IO_NULL			6  

#define     mBuff 			1 // N of Wave Buffers.

typedef struct sample_tag { // one sample
   short leftSample;
   short rightSample;
} SAMPLE;

typedef struct acuBuffTag_IN {
  SAMPLE acuBuff[NSAMPLE_IN];
} ACUBUFF_IN;

typedef struct acuBuffTag_OUT {
  SAMPLE acuBuff[NSAMPLE_OUT];
} ACUBUFF_OUT;

typedef ACUBUFF_IN  FAR *LPACUBUFF_IN;
typedef ACUBUFF_OUT FAR *LPACUBUFF_OUT;

typedef struct wavebuff_tag { // Wave Buffer :
   LPWAVEHDR   lpWaveHdr; 
   HANDLE      hWaveHdr;  // (Lock(hWaveHdr) = lpWaveHdr)
   HANDLE      hWaveData; // (Lock(hWaveData) = lpWaveHdr->lpData)
   UINT        io;   // prep in / prep out / input / output / free
} WAVEBUFF;

         /* data */

extern WAVEBUFF    		wBuff[mBuff];
extern HWAVEOUT    		hWaveOut;  		// OutPut Device
extern HWAVEIN     		hWaveIn;   		// Input Device
extern PCMWAVEFORMAT    PcmWaveFormat;  // Wave Format

//extern HWND        hwndRec;		 // record window handle
//extern HWND        hwndSpDist;

typedef double	FBUFF[NSAMPLE_IN];
typedef FBUFF	FAR  *LPFBUFF;

extern FBUFF sl,sr;
extern HANDLE  hLeftMid,hRightMid, hLeftSample,hRightSample;
//extern short	curChannel,nextChannel;     // clavier (L/R)
 
         /* ===== Procedures ====== */

UINT        checkOk(HWND hWnd, UINT resultCode, UINT ioc );

UINT	    OpenDevice( HWND hWnd, UINT ioc );
VOID	    CloseDevice( HWND hWnd, UINT ioc );

BOOL        prepareIn( HWND hWnd, int nom );
BOOL        prepareOut( HWND hWnd, int nom );
UINT        unprepareIn( HWND hWnd, int nom );
UINT        unprepareOut( HWND hWnd, int nom );  

VOID        InitBuffers( HWND hWnd );
UINT		OpenBuffer( HWND hWnd, int nom, DWORD dwDataSize, DWORD code );
UINT        CloseBuffers( HWND hWnd );

UINT 		PlayMySound( HWND hWnd, int nom );
UINT        RecordSoundOpen( HWND hWnd, int nom );
UINT        RecordSound( HWND hWnd, int nom );

VOID        OutErrorMessage(HWND hWnd, UINT ErrCode, UINT ioc);

// --------------------------------------------------

BOOL FAR PASCAL __export RecordTest(HWND hWnd, unsigned wMsg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export DeviceTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export ScanTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export ChoiceTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);

BOOL FAR PASCAL __export ChooseTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export CheckTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export ScanTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);



// Paint.c

#define fMin 0.00001

//extern	HPEN hDashPen;              /* "---" pen handle   */
//extern	HPEN hDotPen;               /* "..." pen handle   */
//extern	HBRUSH hOldBrush;           /* old brush handle   */
//extern	HBRUSH hRedBrush;           /* red brush handle   */
//extern	HBRUSH hGreenBrush;         /* green brush handle */
//extern	HBRUSH hBlueBrush;          /* blue brush handle  */

//extern 	short roomRel;

int		printW(HWND hDlg, UINT ne, char *s, ...);

VOID	PaintFun(BOOL paintMode, HWND hWnd, double arr[], UINT mArr, UINT id);

VOID	PaintBuff(BOOL paintMode, HWND hWnd, double f[], UINT id);
VOID	PaintRoom(BOOL paintMode, HWND hWnd, UINT id);

VOID	messageWin(HWND hWnd, char *s);

// anal.c

short absD(double co[], short mCo );
short minD(double co[], short mCo );
short maxD(double co[], short mCo );
double minV(double co[], short mCo );
double maxV(double co[], short mCo );
short maxDD(double f[], short mCo );

short 		round(double f);

VOID 		trend( double f[], short m );
VOID 		normirFun( double f[], short m );
VOID 		DoLowPass( HWND hWnd );
VOID		CopyData( int nom );   
//VOID		InitOutData( int nom );   
UINT		InitAnal( HWND hWnd );   
//UINT 		saveBuffers( HWND hWnd );
VOID		InitData( HWND hWnd );
UINT 		clearBuffers( HWND hWnd );

#define MCORR 700

extern double corrL[MCORR];//,corrR[MCORR];

extern short dBugL;
//,dBugR,dSpeakers;

//VOID CalcCorr( LPFBUFF f1, LPFBUFF f2, double c[] );
// MIDI
#define BUFFER_LENGTH 128

DWORD SendString(HWND hWnd, char *s);

// tty.c

int myDialogBox( HWND hWnd, LPCSTR lpszTemplate, FARPROC lpDlgProc);

// Window's.

// Picture & Text & Button.

typedef   struct picture_text_button_tag {
   int dHTop,
       dHBottom,
       dHButton,
       hPMin,
       lPMin,
       dLPicture,
       dHText;
} PICTURE_TEXT_BUTTON_WINDOW;

// Small Window.

typedef   struct small_window_tag {
   char   *szName; // application name
   HWND   hWnd;  // window name
   RECT   L;
   RECT   R;
   RECT   LA;
   RECT   RA;
   double max,min;
   BOOL   On,Cl;
   HBRUSH hBrushR,hBrushG;
   HPEN   hPen;
} SMALL_WINDOW;

extern HANDLE	 ghInst;   // instance handle

VOID PASCAL MakeWaveWin(HWND hWndO, HINSTANCE hInst);

VOID PASCAL MakeLpcWin(HWND hWndO, HINSTANCE hInst);

//extern BOOL waveWinOn, lpcWinOn;
//extern HWND hWaveWnd, hLpcWnd;
//void graphR(HWND hWnd, HDC hdc, RECT rF, double *f, long nOfSamples);

void initSmallWindow(SMALL_WINDOW *sm, char *szWinName);

void clearSmallWindow(SMALL_WINDOW *sm);

void graphAmp(HWND hWnd, HDC hdc, RECT rF);

void graphR(HWND hWnd, HDC hdc, RECT rF, double *f,
            long nOfSamples, HPEN hPen, HBRUSH hBrush);

extern SMALL_WINDOW waveWin,lpcWin,mainWin;

//== scnpaint

VOID sortAFM( void );

VOID orderAFM( void );

void marker(BOOL paintMode, HWND hWnd, long pM, long nOfSamples, UINT id);

// scan.c

BOOL FAR ProcessCOMMNotification ( HWND hWnd, WORD wParam, LONG lParam );

VOID resizePicWin ( HWND hWnd, PICTURE_TEXT_BUTTON_WINDOW *picWin );

VOID initResizePicWin ( HWND hWnd, PICTURE_TEXT_BUTTON_WINDOW *picWin );

// loc1.c

// midi.c

// extern  char  aszBuffer[BUFFER_LENGTH];

DWORD SendString(HWND hWnd, char *s);

VOID	midiInit( HWND hWnd, char *filename, char *channel );

VOID	midiPlay( HWND hWnd, char *channel );

VOID	midiClose( HWND hWnd, char *channel );

UINT	midiStatus( HWND hWnd, char *channel );


/* eof */

