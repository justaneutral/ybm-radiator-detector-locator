#include "cinc.h"

//#include "stdio.h"
//#include "string.h"
//#include "math.h"

// __________________ Gra Win __________________

static BOOL ClOn = FALSE;


VOID InitGraWin( LPFUNWIN lpFW, DOUBLEARRAY d, LPCSTR s ) {
  lpFW->On = FALSE;
  //lpFW->Cl = FALSE;
  lpFW->hWnd = NULL;
  lpFW->szName = s;
  lpFW->d = d;  
}

VOID PASCAL MakeGraWin(HWND hWndO, LPFUNWIN lpFW, short nWin)
{
    WNDCLASS    wc;
    RECT 		rW;
    UINT	    drX = 10,
                drY = 10;
    LPCSTR		fClassName = "funwincls";            

    if ( lpFW->d == NULL ) {
      Message(hWndO, " Null ptr in Make Win");
      CommonError = TRUE;
      return;
    }  
    
    /* Define and register a window class for the main window.
     */
    
    if ( lpFW->On )
      return;
      
    if ( !ClOn ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION); //hInst, waveWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = fClassName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = ghInst;
        wc.style          = 0;
        wc.lpfnWndProc    = GraWindowProc;
        wc.cbWndExtra     = sizeof(LPFUNWIN);
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        ClOn = TRUE;  
    }

    GetWindowRect(hWndO,&rW);
    
    lpFW->hWnd = CreateWindow (fClassName,           // class name
                            lpFW->szName,            // caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			
			    			// WS_OVERLAPPEDWINDOW,  // style bits
                            // rW.left+(rW.right-rW.left)/2,// x position
                            
                            0,
                            rW.bottom+               // y position
							(GetSystemMetrics(SM_CYSCREEN)/5)*nWin,

  	    					GetSystemMetrics(SM_CXSCREEN)/2,
							GetSystemMetrics(SM_CYSCREEN)/5,

                            // (rW.right-rW.left)/2, //WMAIN_DX, // x size
                            // (rW.bottom-rW.top),   //WMAIN_DY, // y size
                            (HWND)hWndO,             // parent window
                            (HMENU)NULL,             // use class menu
                            (HANDLE)ghInst,          // instance handle
                            (LPSTR)NULL              // no params to pass on
                           );

    SendMessage(lpFW->hWnd, WM_COMMAND,101,(long)lpFW);

    GetClientRect(lpFW->hWnd,&rW);
    
    lpFW->RF.left = drX;    
    lpFW->RF.right = rW.right-drX;
    
    lpFW->RF.top = drY;
    lpFW->RF.bottom = rW.bottom-drY;
    
    //lpFW->On = TRUE;
 
    ShowWindow(lpFW->hWnd,SW_SHOWNORMAL);   
}


VOID drawGraWin(HDC hdc, LPFUNWIN lpFW) {
  HBRUSH hOldBrush;

  hOldBrush = SelectObject(hdc, pen.bGray);  
  Rectangle(hdc,lpFW->RF.left,lpFW->RF.top,lpFW->RF.right,lpFW->RF.bottom);
  SelectObject(hdc, hOldBrush);
}


VOID resizeGraWin( HWND hWnd, HDC hdc, LPFUNWIN lpFW ) {
  RECT 		rW;
  UINT	    drX = 10,
            drY = 10;
            
  HBRUSH 	hBrushW,hOldBrush;
  HPEN   	hPenW,hOldPen;
    

  hPenW = GetStockObject(WHITE_PEN);
  hBrushW = GetStockObject(WHITE_BRUSH);

  GetClientRect(hWnd,&rW);
    
  hOldPen = SelectObject(hdc,hPenW);
  hOldBrush = SelectObject(hdc,hBrushW);
    
  Rectangle(hdc,rW.left,rW.top,rW.right,rW.bottom);
    
  SelectObject(hdc,hOldPen);
  SelectObject(hdc,hOldBrush);
    
  lpFW->RF.left = drX;    
  lpFW->RF.right = rW.right-drX;
    
  lpFW->RF.top = drY;
  lpFW->RF.bottom = rW.bottom-drY;
}


VOID drawGraWin1(HDC hdc, LPFUNWIN lpFW) {
  //HBRUSH hOldBrush; //, hBrush, hBrushA;

  /*
  hOldBrush = SelectObject(hdc,pen.bGray);
  
  Rectangle(hdc,lpFW->RF.left,lpFW->RF.top,lpFW->RF.right,lpFW->RF.bottom);

  SelectObject(hdc, hOldBrush);
  */
  
  drawF(hdc, &(lpFW->RF), lpFW->d);
}



LONG FAR PASCAL __export GraWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  LPFUNWIN lpFW;
  
  lpFW = (LPFUNWIN)GetWindowLong(hWnd,0);
    
  switch (msg) {
  
    case WM_INITDIALOG:
      return TRUE;
      
    case WM_DESTROY:
      if ( lpFW )
        lpFW->On = FALSE;
	    //PostQuitMessage(0);
	  return 0;
	    
	case WM_PAINT:
      if ( lpFW ) {
        hdc = BeginPaint(hWnd,&ps);
	    drawGraWin(hdc,lpFW);
    	EndPaint(hWnd,&ps);
      }	
	  return 0;    

    case WM_SIZE: 
      if ( lpFW ) {
        hdc = GetDC(hWnd);
        resizeGraWin(hWnd,hdc,lpFW);
  	    drawGraWin(hdc,lpFW);
	    ReleaseDC(hWnd,hdc);
	  }  
      break;
      
    case WM_SYSCOMMAND:
      switch (wParam)
        {
        }
      break;
                
    case WM_COMMAND:
        switch (wParam)
        {
        case 101:
          SetWindowLong(hWnd,0,lParam);
          lpFW = (LPFUNWIN)lParam;
          hdc = GetDC(hWnd);
          resizeGraWin(hWnd,hdc,lpFW);
     	  drawGraWin(hdc,lpFW);
     	  ReleaseDC(hWnd,hdc);
     	  lpFW->On = TRUE;
          break;

        case 100:
          if ( lpFW ) {
            hdc = GetDC(hWnd);
            drawGraWin1(hdc,lpFW);
	        ReleaseDC(hWnd,hdc);
	      }
	      break;
	        
        /*
        case 102:
          if ( lpFW ) {
            hdc = GetDC(hWnd);
            drawDirection(hdc,lpFW);
            ReleaseDC(hWnd,hdc);
          }  
          break;
        */  
    	}
        return( 0L );
    }

    return DefWindowProc(hWnd,msg,wParam,lParam);
}

