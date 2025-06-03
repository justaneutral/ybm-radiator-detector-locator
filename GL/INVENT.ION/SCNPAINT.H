short 		round(double f);

extern struct scanScaleTag {
  UINT nxtV;     // n of next sample
  long f1;       // frequencies
  long f2;
  long fStep;    // frequency step
  
  RECT r;        // current sizes of graph.
  RECT rMin;     // min sizes. 
  
  HGLOBAL hV;    // samples
  char FAR *pV;  // -"-
  UINT nV;       // Max N of samples.
  UINT nMax;     // n of Max
} scS;
  
struct freqMaxTag {
  UINT ac;     // n of next sample
  char v;      // value
  long f;      // frequency
  double pr;   // prior.
};

typedef struct freqMaxTag FAR *arrFreqMax;
extern HGLOBAL    hAFM;    // array of freq max's.
extern arrFreqMax afm;
extern int scanCount;

UINT	initFreqData( long f1, long f2, long fStep );

UINT	clearFreqPaint( void );        // unprepare

void    graphic(BOOL paintMode, HWND hWnd, char *f, long nOfSamples, UINT id);

UINT	repaintFreqScale(HWND hWnd, UINT mode);      // Paint / ReDraw

UINT	addScanSample(HWND hWnd, char v); // one reading

int		printW(HWND hDlg, UINT ne, char *s, ...);

//==

VOID SaveMaxFreq(HWND hWnd);
VOID SaveMaxFreqAFM(HWND hWnd);
UINT LoadMaxFreq(HWND hWnd);
BOOL ClearAFMPtr(HWND hWnd);
