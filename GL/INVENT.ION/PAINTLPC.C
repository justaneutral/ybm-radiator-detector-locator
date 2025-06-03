#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include "sound.h"
#include "fft.h"

LONG FAR PASCAL __export LpcWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

SMALL_WINDOW lpcWin;

VOID PASCAL MakeLpcWin(HWND hWndO, HINSTANCE hInst /* HINSTANCE hPrev, LPSTR szCmdLine, int cmdShow */)
{
    WNDCLASS    wc;
    RECT 		rW;
    UINT	    drX = 10,drY = 10;

    /* Define and register a window class for the main window. */
    
    if (lpcWin.On == TRUE)
      return;
       
    if (lpcWin.Cl == FALSE /*!hPrev*/)
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(hInst, lpcWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = lpcWin.szName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = hInst;
        wc.style          = 0;
        wc.lpfnWndProc    = LpcWndProc;
        wc.cbWndExtra     = 0;
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        lpcWin.Cl = TRUE;  
    }

    /* Create and show the main window.  */
     
    GetWindowRect(hWndO,&rW);
    
    lpcWin.hWnd = CreateWindow (lpcWin.szName,              // class name
                            lpcWin.szName,              // caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			// WS_OVERLAPPEDWINDOW,    // style bits
                            rW.left+(rW.right-rW.left)/2,
                            //CW_USEDEFAULT,        // x position
                            rW.bottom,//CW_USEDEFAULT,      // y position
                            (rW.right-rW.left)/2, //WMAIN_DX, // x size
                            (rW.bottom-rW.top)*2/3,   //WMAIN_DY, // y size
                            (HWND)hWndO,            // parent window
                            (HMENU)NULL,            // use class menu
                            (HANDLE)hInst,          // instance handle
                            (LPSTR)NULL             // no params to pass on
                           );

    GetClientRect(lpcWin.hWnd,&rW);
    
    lpcWin.L.left = drX;
    lpcWin.R.left = drX;
    
    lpcWin.L.right = rW.right-drX;
    lpcWin.R.right = rW.right-drX;
    
    lpcWin.L.top = drY;
    lpcWin.L.bottom = (rW.bottom-drY)/2;
    
    lpcWin.R.top = lpcWin.L.bottom+drY;
    lpcWin.R.bottom = rW.bottom-drY;

    ShowWindow(lpcWin.hWnd,SW_SHOWNORMAL /*cmdShow*/);
    
    lpcWin.On = TRUE;
}

VOID drawLpcWin(HDC hdc) {

  HBRUSH hOldBrush; 

  hOldBrush = SelectObject(hdc, lpcWin.hBrushG);
  Rectangle(hdc,lpcWin.L.left,lpcWin.L.top,lpcWin.L.right,lpcWin.L.bottom);
  Rectangle(hdc,lpcWin.R.left,lpcWin.R.top,lpcWin.R.right,lpcWin.R.bottom);
  SelectObject(hdc, hOldBrush);
}

VOID resizeLpcWin ( HWND hWnd, HDC hdc ) {
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
    
    
    lpcWin.L.left = drX;
    lpcWin.R.left = drX;
    
    lpcWin.L.right = rW.right-drX;
    lpcWin.R.right = rW.right-drX;
    
    lpcWin.L.top = drY;
    lpcWin.L.bottom = (rW.bottom-drY)/2;
    
    lpcWin.R.top = lpcWin.L.bottom+drY;
    lpcWin.R.bottom = rW.bottom-drY;
}


/* WndProc - Lpc window procedure function. */
 
LONG FAR PASCAL __export LpcWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;  
    
    switch (msg)
    {
    case WM_DESTROY:
      lpcWin.On = FALSE;
	  return 0;
	    
    case WM_SIZE: 
          hdc = GetDC(hWnd);
          resizeLpcWin(hWnd,hdc);
     	  drawLpcWin(hdc);
		  ReleaseDC(hWnd,hdc);
         break;

	case WM_PAINT:
	  hdc = BeginPaint(hWnd,&ps);
	  drawLpcWin(hdc);
	  EndPaint(hWnd,&ps);
	  return 0;    
      
    case WM_SYSCOMMAND:
        switch (wParam)
        {
        }
        break;
                
    case WM_COMMAND:
        switch (wParam)
        {
        case 2:
          hdc = GetDC(hWnd);
          
          if ( lParam == 0 ) {
		    graphR(hWnd, hdc, lpcWin.L, fftX, mFftH,lpcWin.hPen,lpcWin.hBrushG);
		    graphR(hWnd, hdc, lpcWin.R, fftY, mFftH,lpcWin.hPen,lpcWin.hBrushG);
		  }
		  else {
		    graphR(hWnd, hdc, lpcWin.L, corrL, MCORR,lpcWin.hPen,lpcWin.hBrushG);
		    //graphR(hWnd, hdc, lpcWin.R, corrR, MCORR,lpcWin.hPen,lpcWin.hBrushG);
		  }
		  
		  ReleaseDC(hWnd,hdc);
          
          return 0;
    	}
        return( 0L );
    }

    return DefWindowProc(hWnd,msg,wParam,lParam);
}


