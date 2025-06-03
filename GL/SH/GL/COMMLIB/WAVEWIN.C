#include "cinc.h"

// Copy buffer -> sr для стерео 16.

VOID copyWave( waveBuffer FAR *pb ) 
{
  oneSample FAR *ab;
  long i,m,m1;
  
  if ( error )
    return;
    
  if ( ( sl == NULL ) or ( sr == NULL ) or (pb == NULL) ) {
    error = TRUE;
    return;
  }  

  m1 = getMemDC(sl)->m;
  m = (pb->WaveHdr.dwBufferLength)/4;
  
  if ( ( m != m1 ) or ( m <= 0 ) ) {
    error = TRUE;
    return;
  }  

  ab = (oneSample FAR *)(pb->WaveHdr.lpData);
  
  for ( i = 0; i < m; i++ ) 
    sr[i] = ab[i].leftSample;

  for ( i = 0; i < m; i++ ) 
    sl[i] = ab[i].rightSample;
}

//____________________________________________________
//
//                   окно "Line in".
//____________________________________________________
//

HWND hWaveWnd = NULL;

static struct {
  HWND hWnd;
  LPCSTR szName;
  BOOL   ClassOn; // признак того, что оконный класс 
                  // уже зарегестрирован.
  RECT   L;
  RECT   R;
  RECT   LA;
  RECT   RA;
} waveWin = { NULL, "Line In", FALSE };

DOUBLEARRAY 
  sl = NULL,
  sr = NULL; // левый и правый каналы, их то мы и рисуем

LONG FAR PASCAL __export waveWinProc(HWND, UINT, WPARAM, LPARAM);

// оконная функция
                              
// ____________________ wave Win ____________________

/*
BOOL waveOn( void ) {
  if ( waveWin.hWnd == NULL )
    return FALSE;
  else
    return TRUE;  
}
*/

static VOID calculateWaveWinRect( void ) {
  RECT 		rW;
  UINT	    drX = 3, drY = 3;
    
  if ( waveWin.hWnd == NULL )
    return;

  GetClientRect(waveWin.hWnd,&rW);
    
  waveWin.L.left = drX*6;
  waveWin.R.left = drX*6;
    
  waveWin.L.right = rW.right-drX;
  waveWin.R.right = rW.right-drX;
    
  waveWin.L.top = drY;
  waveWin.L.bottom = (rW.bottom-drY)/2;
    
  waveWin.R.top = waveWin.L.bottom+drY;
  waveWin.R.bottom = rW.bottom-drY;
    
  // ==
    
  waveWin.LA.left = drX;
  waveWin.RA.left = drX;
    
  waveWin.LA.right = drX*5;
  waveWin.RA.right = drX*5;
    
  waveWin.LA.top = drY;
  waveWin.LA.bottom = (rW.bottom-drY)/2;
    
  waveWin.RA.top = waveWin.L.bottom+drY;
  waveWin.RA.bottom = rW.bottom-drY;
}

  
VOID resizeWaveWin ( HDC hdc ) {
  RECT 		rW;

  HBRUSH hBrushW,hOldBrush;
  HPEN   hPenW,hOldPen;
    
  if ( waveWin.hWnd == NULL )
    return;

  hPenW = GetStockObject(WHITE_PEN);
  hBrushW = GetStockObject(WHITE_BRUSH);

  GetClientRect(waveWin.hWnd,&rW);
    
  hOldPen = SelectObject(hdc,hPenW);
  hOldBrush = SelectObject(hdc,hBrushW);
    
  Rectangle(hdc,rW.left,rW.top,rW.right,rW.bottom);
    
  SelectObject(hdc,hOldPen);
  SelectObject(hdc,hOldBrush);
  
  calculateWaveWinRect();
}


VOID makeWaveWin(HWND hWndO, short x0, short y0, short dx, short dy) {
  WNDCLASS  wc;
  LPCSTR    className = "waveWinClass";

  if ( error )
    return;
    
  if ( (sr == NULL) or (sl == NULL) ) {
    errormessage(hWndO, "sr / sl init error");
    return;
  }    
      
  /* Define and register a window class for the main window.
  */
  
  if ( waveWin.hWnd != NULL )
    return;
      
  if ( waveWin.ClassOn == FALSE ) // !hPrev
  {
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION); 
    wc.lpszMenuName   = (LPSTR)NULL; 
    wc.lpszClassName  = className;
    wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); 
    wc.hInstance      = ghInst;
    wc.style          = 0;
    wc.lpfnWndProc    = waveWinProc;
    wc.cbWndExtra     = 0;
    wc.cbClsExtra     = 0;

    if (!RegisterClass(&wc)) {
      errormessage(hWndO,"Register Class Error");
      return;
    }  

    waveWin.ClassOn = TRUE;  
  }

  hWaveWnd = waveWin.hWnd = CreateWindow (className,  
                    waveWin.szName,         // caption
	       			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
	      			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
                    x0, y0, dx, dy, 
                    (HWND)hWndO,            // parent window
                    (HMENU)NULL,            // use class menu
                    (HANDLE)ghInst,         // instance handle
                    (LPSTR)NULL             // no params to pass on
                    );

  if ( waveWin.hWnd == NULL ) {
    errormessage(hWndO," Create Window Error ");
    return;
  }  

  calculateWaveWinRect();
  
  ShowWindow(waveWin.hWnd,SW_SHOWNORMAL /*cmdShow*/);
}

/*
static VOID drawWaveRect(HDC hdc) {
  HBRUSH hOldBrush;

  if ( waveWin.On != TRUE )
    return;

  hOldBrush = SelectObject(hdc, pen.bGray);
  
  Rectangle(hdc,waveWin.L.left,waveWin.L.top,waveWin.L.right,waveWin.L.bottom);
  Rectangle(hdc,waveWin.R.left,waveWin.R.top,waveWin.R.right,waveWin.R.bottom);

  SelectObject(hdc, pen.bGreen);

  Rectangle(hdc,waveWin.LA.left,waveWin.LA.top,waveWin.LA.right,waveWin.LA.bottom);
  Rectangle(hdc,waveWin.RA.left,waveWin.RA.top,waveWin.RA.right,waveWin.RA.bottom);
  
  SelectObject(hdc, hOldBrush);
}
*/


static VOID drawWave(HDC hdc) {
  HBRUSH hOldBrush;
  WINDOWPLACEMENT wndpl; 

  if ( waveWin.hWnd == NULL )
    return;
    
  if ( GetWindowPlacement(waveWin.hWnd,&wndpl) ) {
    if ( (wndpl.showCmd & SW_SHOWMINIMIZED) != 0 )
      return;
  }
  
  hOldBrush = SelectObject(hdc,pen.bGray);
  
  Rectangle(hdc,waveWin.L.left,waveWin.L.top,waveWin.L.right,waveWin.L.bottom);
  Rectangle(hdc,waveWin.R.left,waveWin.R.top,waveWin.R.right,waveWin.R.bottom);

  SelectObject(hdc, pen.bGreen);

  Rectangle(hdc,waveWin.LA.left,waveWin.LA.top,waveWin.LA.right,waveWin.LA.bottom);
  Rectangle(hdc,waveWin.RA.left,waveWin.RA.top,waveWin.RA.right,waveWin.RA.bottom);
  
  SelectObject(hdc, hOldBrush);

  if ( (sl == NULL) or (sr == NULL) )
    return;
    
  drawF(hdc, &waveWin.L, sl);
  drawAmp(hdc, &waveWin.LA, sl);
  drawF(hdc, &waveWin.R, sr);
  drawAmp(hdc, &waveWin.RA, sr);
}


VOID putWave( void ) {
  HDC hdc;  
  
  if ( waveWin.hWnd == NULL )
    return;

  hdc = GetDC(waveWin.hWnd);
  drawWave(hdc);
  ReleaseDC(waveWin.hWnd,hdc);
}


LONG FAR PASCAL __export waveWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;  
    
    switch (msg)
    {
    case WM_DESTROY:
      hWaveWnd = waveWin.hWnd = NULL;
	  return 0;
	    
	case WM_PAINT:
	  hdc = BeginPaint(hWnd,&ps);
	  drawWave(hdc);
	  EndPaint(hWnd,&ps);
	  return 0;    

    case WM_SIZE: 
      hdc = GetDC(hWnd);
      resizeWaveWin(hdc);
      drawWave(hdc);
	  ReleaseDC(hWnd,hdc);
      return 0;
    }  
    return DefWindowProc(hWnd,msg,wParam,lParam);
}
