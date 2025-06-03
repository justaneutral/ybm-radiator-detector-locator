#include "cinc.h"

// для Draw Dir

#include "stdio.h"
#include "string.h"
#include "math.h"

//#include "resource.h"

//____________________________________________________
//
// окно "Line in".
//____________________________________________________
//
/*
struct WaveWindowTag
{
   char   *szName; // application name
   HWND   hWnd;  // window name
   RECT   L;
   RECT   R;
   RECT   LA;
   RECT   RA;
   double max,min;
   BOOL   On;
}
*/
FUNWIN waveWin;

struct WaveWindowRectTag
{
   //char   *szName; // application name
   //HWND   hWnd;  // window name
   RECT   L;
   RECT   R;
   RECT   LA;
   RECT   RA;
   //double max,min;
   //BOOL   On;
}  waveWinRect;


/*
typedef struct FunWindowTag {
   LPCSTR szName; // application name
   HWND   hWnd;  // window name
   RECT   RF;
   double max,min;
   BOOL   On,Cl;
   arrDC  *f;
} FUNWIN;

typedef FUNWIN FAR *LPFUNWIN;

FUNWIN fw;
LPFUNWIN newFW; // для передачи окну указателя на его данные.
*/

//LONG FAR PASCAL __export WaveWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//LONG FAR PASCAL __export FunWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

DOUBLEARRAY sl,sr;

static BOOL WaveClOn = FALSE; // признак того, что оконный класс 
                              // уже зарегестрирован.
                              
// ____________________ Wave Win ____________________

VOID InitWaveWindow( void ) {
  waveWin.On = FALSE;
  //waveWin.Cl = FALSE;
  waveWin.hWnd = NULL;
  waveWin.szName = "Line In";
}  

VOID PASCAL MakeWaveWin(HWND hWndO)
{
    WNDCLASS    wc;
    RECT 		rW;
    UINT	    drX = 10,
                drY = 10;

    /* Define and register a window class for the main window.
     */
    if ( waveWin.On == TRUE )
      return;
      
    if ( WaveClOn == FALSE ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION); //hInst, waveWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = waveWin.szName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = ghInst;
        wc.style          = 0;
        wc.lpfnWndProc    = WaveWindowProc;
        wc.cbWndExtra     = 0;
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        WaveClOn = TRUE;  
    }

    GetWindowRect(hWndO,&rW);
    
    waveWin.hWnd = CreateWindow (waveWin.szName,        // class name
                            waveWin.szName,         // caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			// WS_OVERLAPPEDWINDOW, // style bits
                            //rW.left, //CW_USEDEFAULT, // x position
                            0,
                            rW.bottom,              // y position

  	    					GetSystemMetrics(SM_CXSCREEN)/2,
							GetSystemMetrics(SM_CYSCREEN)/5,
                            
                            //(rW.right-rW.left)/2,   //WMAIN_DX, // x size
                            //(rW.bottom-rW.top),   //WMAIN_DY, // y size
                            (HWND)hWndO,            // parent window
                            (HMENU)NULL,            // use class menu
                            (HANDLE)ghInst,          // instance handle
                            (LPSTR)NULL             // no params to pass on
                           );

    GetClientRect(waveWin.hWnd,&rW);
    
    waveWinRect.L.left = drX+drX+drX/2;
    waveWinRect.R.left = drX+drX+drX/2;
    
    waveWinRect.L.right = rW.right-drX;
    waveWinRect.R.right = rW.right-drX;
    
    waveWinRect.L.top = drY;
    waveWinRect.L.bottom = (rW.bottom-drY)/2;
    
    waveWinRect.R.top = waveWinRect.L.bottom+drY;
    waveWinRect.R.bottom = rW.bottom-drY;
    
    // ==
    
    waveWinRect.LA.left = drX;
    waveWinRect.RA.left = drX;
    
    waveWinRect.LA.right = drX*2;
    waveWinRect.RA.right = drX*2;
    
    waveWinRect.LA.top = drY;
    waveWinRect.LA.bottom = (rW.bottom-drY)/2;
    
    waveWinRect.RA.top = waveWinRect.L.bottom+drY;
    waveWinRect.RA.bottom = rW.bottom-drY;

    ShowWindow(waveWin.hWnd,SW_SHOWNORMAL /*cmdShow*/);
    
    waveWin.On = TRUE;
}


VOID drawWave(HDC hdc) {

  HBRUSH hOldBrush;

  hOldBrush = SelectObject(hdc, pen.bGray);
  
  Rectangle(hdc,waveWinRect.L.left,waveWinRect.L.top,waveWinRect.L.right,waveWinRect.L.bottom);
  Rectangle(hdc,waveWinRect.R.left,waveWinRect.R.top,waveWinRect.R.right,waveWinRect.R.bottom);

  SelectObject(hdc, pen.bGreen);

  Rectangle(hdc,waveWinRect.LA.left,waveWinRect.LA.top,waveWinRect.LA.right,waveWinRect.LA.bottom);
  Rectangle(hdc,waveWinRect.RA.left,waveWinRect.RA.top,waveWinRect.RA.right,waveWinRect.RA.bottom);
  
  SelectObject(hdc, hOldBrush);
}

VOID resizeWaveWin ( HWND hWnd, HDC hdc ) {
    RECT 		rW;
    UINT	    drX = 10,
                drY = 10;
    HBRUSH hBrushW,hOldBrush;
    HPEN   hPenW,hOldPen;
    
    hPenW = GetStockObject(WHITE_PEN);
    hBrushW = GetStockObject(WHITE_BRUSH);

    GetClientRect(hWnd,&rW);
    
    hOldPen = SelectObject(hdc,hPenW);
    hOldBrush = SelectObject(hdc,hBrushW);
    
    Rectangle(hdc,rW.left,rW.top,rW.right,rW.bottom);
    
    SelectObject(hdc,hOldPen);
    SelectObject(hdc,hOldBrush);
    
    waveWinRect.L.left = drX+drX+drX/2;
    waveWinRect.R.left = drX+drX+drX/2;
    
    waveWinRect.L.right = rW.right-drX;
    waveWinRect.R.right = rW.right-drX;
    
    waveWinRect.L.top = drY;
    waveWinRect.L.bottom = (rW.bottom-drY)/2;
    
    waveWinRect.R.top = waveWinRect.L.bottom+drY;
    waveWinRect.R.bottom = rW.bottom-drY;
    
    // ==
    
    waveWinRect.LA.left = drX;
    waveWinRect.RA.left = drX;
    
    waveWinRect.LA.right = drX*2;
    waveWinRect.RA.right = drX*2;
    
    waveWinRect.LA.top = drY;
    waveWinRect.LA.bottom = (rW.bottom-drY)/2;
    
    waveWinRect.RA.top = waveWinRect.L.bottom+drY;
    waveWinRect.RA.bottom = rW.bottom-drY;
}

VOID drawWave1(HDC hdc) {
  HBRUSH hOldBrush; //, hBrush, hBrushA;

  hOldBrush = SelectObject(hdc,pen.bGray);
  
  Rectangle(hdc,waveWinRect.L.left,waveWinRect.L.top,waveWinRect.L.right,waveWinRect.L.bottom);
  Rectangle(hdc,waveWinRect.R.left,waveWinRect.R.top,waveWinRect.R.right,waveWinRect.R.bottom);

  SelectObject(hdc, pen.bGreen);

  Rectangle(hdc,waveWinRect.LA.left,waveWinRect.LA.top,waveWinRect.LA.right,waveWinRect.LA.bottom);
  Rectangle(hdc,waveWinRect.RA.left,waveWinRect.RA.top,waveWinRect.RA.right,waveWinRect.RA.bottom);
  
  SelectObject(hdc, hOldBrush);

  drawF(hdc, &waveWinRect.L, sl);
  drawAmp(hdc, &waveWinRect.LA, sl);
  drawF(hdc, &waveWinRect.R, sr);
  drawAmp(hdc, &waveWinRect.RA, sr);
}

LONG FAR PASCAL __export WaveWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;  
    
    switch (msg)
    {
    case WM_INITDIALOG:
      waveWin.On = TRUE;
      return TRUE;
      
    case WM_DESTROY:
      waveWin.On = FALSE;
	    //PostQuitMessage(0);
	  return 0;
	    
	case WM_PAINT:
	  hdc = BeginPaint(hWnd,&ps);
	  drawWave(hdc);
	  EndPaint(hWnd,&ps);
	  return 0;    

    case WM_SIZE: 
          hdc = GetDC(hWnd);
          resizeWaveWin(hWnd,hdc);
     	  drawWave(hdc);
		  ReleaseDC(hWnd,hdc);
         break;
      
    case WM_SYSCOMMAND:
        switch (wParam)
        {
        }
        break;
                
    case WM_COMMAND:
        switch (wParam)
        {
        case 100:
          hdc = GetDC(hWnd);
          drawWave1(hdc);
		  ReleaseDC(hWnd,hdc);
          return 0;
    	}
        return( 0L );
    }

    return DefWindowProc(hWnd,msg,wParam,lParam);
}

