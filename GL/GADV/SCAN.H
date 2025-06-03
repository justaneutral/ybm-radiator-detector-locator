typedef short vFreq;
typedef long freq;
typedef struct { freq f; vFreq v; } FreqVal;

extern FreqVal tV;
extern freq  /*FirstFreq, LastFreq,*/ BigStep, SmallStep;

typedef struct 
  FreqMaxTag { 
      FreqVal  fMid, fMin, fMax;
      double power;   // приоритет
  } FreqMaxRec;
  
#define mFreqMax 1500 // Размер массива для максимумов

//________________________________________________
//                                                           
//                   ScanMax.c                            
//________________________________________________

typedef struct 
  FreqTabTag { 
    long nMax;                  // число максимумов.
    FreqMaxRec FAR *FreqMax;    // указатель на таблицу максимумов.
    freq  FirstFreq,
          LastFreq;
  } FreqTab;

typedef FreqTab FAR *LPFreqTab;

extern FreqTab CurTab;               

BOOL FreqScaleOn( LPFreqTab, BOOL St );

VOID SaveFreqScale(HWND hWnd, LPCSTR s, LPFreqTab);

VOID LoadFreqScale(HWND hWnd, LPCSTR s, LPFreqTab);

VOID sortFreqMax( LPFreqTab );


VOID ClearMaxScan( LPFreqTab );

VOID CommSendStr( HWND hWnd );

VOID freqReport(HWND hWnd);

// __________________ Scale Win ________________


extern FUNWIN fwScale;

VOID InitScaleWin( LPCSTR s );

VOID PASCAL MakeScaleWin( HWND hWndO );

LONG FAR PASCAL __export ScaleWindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

VOID drawFreqMax(HDC rDC, long m1, long m2);

//VOID drawPowerMax(HDC rDC, long m1, long m2);

VOID reDrawScale( void );

VOID ClearFreqScale(HDC rDC);


// __________________ Power Scale Win ________________

extern FUNWIN fwPowerScale;

VOID InitPowerScaleWin ( LPCSTR s );

VOID PASCAL MakePowerScaleWin (HWND hWndO);

//LONG FAR PASCAL __export ScaleWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//VOID drawPowerMax(HDC rDC, long m1, long m2);

VOID PutPowerMarker (long NewPos);

BOOL drawPowerMax (HDC rDC, long m1, long m2);

VOID reDrawPowerScale ( void );

VOID ClearPowerScale (HDC rDC);

LONG FAR PASCAL __export PowerScaleWindowProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
