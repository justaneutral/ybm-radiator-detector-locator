#include "cinc.h"
#include "gant.h"

//____________________________________________________
//
//                 Окно из nFun функций.
//____________________________________________________
//

FUNWIN pFunWin = { NULL, "Amp", NULL };

static RECT pr[nFun];

DOUBLEARRAY pd[nFun] = {NULL,NULL};

static BOOL pFunClOn = FALSE; // признак того, что оконный класс 
                              // уже зарегестрирован.

LONG FAR PASCAL __export pFunWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
                              
// ____________________ pFun Win ____________________

static VOID getpFunWinRect( HWND hWnd ) {
  RECT 		rW;
  UINT	    drX = 3, drY = 3, i, h;
    
  if ( pFunWin.hWnd == NULL )
    return;

  GetClientRect(hWnd,&rW);
  
  h = (rW.bottom-drY*(nFun+1))/nFun;
    
  loop(i,nFun) {
    pr[i].top = drY+(h+drY)*i;
    pr[i].bottom = pr[i].top+h;
    pr[i].left = drX;
    pr[i].right = rW.right-drX;
  }  
}

  
static VOID resizepFunWin ( HWND hWnd, HDC hdc ) {
  RECT 	rW;

  HBRUSH hBrushW,hOldBrush;
  HPEN   hPenW,hOldPen;
    
  if ( pFunWin.hWnd == NULL )
    return;

  hPenW = GetStockObject(WHITE_PEN);
  hBrushW = GetStockObject(WHITE_BRUSH);

  GetClientRect(hWnd,&rW);
    
  hOldPen = SelectObject(hdc,hPenW);
  hOldBrush = SelectObject(hdc,hBrushW);
    
  Rectangle(hdc,rW.left,rW.top,rW.right,rW.bottom);
    
  SelectObject(hdc,hOldPen);
  SelectObject(hdc,hOldBrush);
  
  getpFunWinRect(hWnd);
}


VOID makepFunWin(HWND hWndO, short x0, short y0, short dx, short dy)
{
  WNDCLASS    wc;
  LPSTR classname = "pFunWin";

  if ( error )
    return;
      
  /* Define and register a window class for the main window.
  */
  if ( pFunWin.hWnd != NULL )
    return;
      
  if ( pFunClOn == FALSE ) // !hPrev
  {
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION);
    wc.lpszMenuName   = (LPSTR)NULL; 
    wc.lpszClassName  = classname;
    wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); 
    wc.hInstance      = ghInst;
    wc.style          = 0;
    wc.lpfnWndProc    = pFunWindowProc;
    wc.cbWndExtra     = 0;
    wc.cbClsExtra     = 0;

    if (!RegisterClass(&wc))
      return;

    pFunClOn = TRUE;  
  }

  pFunWin.hWnd = CreateWindow (classname,    // class name
                    pFunWin.szName,         // caption
	       			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
	      			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
                    x0, y0, dx, dy, 
                    (HWND)hWndO,            // parent window
                    (HMENU)NULL,            // use class menu
                    (HANDLE)ghInst,         // instance handle
                    (LPSTR)NULL             // no params to pass on
                    );

  if ( pFunWin.hWnd == NULL ) 
    return;
  getpFunWinRect(pFunWin.hWnd);
  ShowWindow(pFunWin.hWnd,SW_SHOWNORMAL /*cmdShow*/);
}


static VOID putpFunH(HDC hdc, int i) {
  HBRUSH hOldBrush;

  if ( pFunWin.hWnd == NULL )
    return;
    
  hOldBrush = SelectObject(hdc, pen.bGray);
  
  Rectangle(hdc,pr[i].left,pr[i].top,pr[i].right,pr[i].bottom);

  SelectObject(hdc, hOldBrush);

  if ( pd[i] == NULL) 
    return;
    
  drawF(hdc, &(pr[i]),pd[i]);
}


VOID putpFun(int i) {
  HDC hdc;  
  WINDOWPLACEMENT wndpl; 

  if ( pFunWin.hWnd == NULL )
    return;

  if ( GetWindowPlacement(pFunWin.hWnd,&wndpl) ) {
    if ( (wndpl.showCmd & SW_SHOWMINIMIZED) != 0 )
      return;
  }
    
  hdc = GetDC(pFunWin.hWnd);

  putpFunH(hdc,i);
  ReleaseDC(pFunWin.hWnd,hdc);
}


static VOID putpFunAll(HDC hdc) {
  int i;
  
  loop(i,nFun)
    putpFunH(hdc,i);
}  


LONG FAR PASCAL __export pFunWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;  
    
    switch (msg)
    {
    case WM_DESTROY:
      pFunWin.hWnd = NULL;
	  return 0;
	    
	case WM_PAINT:
	  hdc = BeginPaint(hWnd,&ps);
	  putpFunAll(hdc);
	  EndPaint(hWnd,&ps);
	  return 0;    

    case WM_SIZE: 
      hdc = GetDC(hWnd);
      resizepFunWin(hWnd,hdc);
      putpFunAll(hdc);
	  ReleaseDC(hWnd,hdc);
	  return 0;    
    }

    return DefWindowProc(hWnd,msg,wParam,lParam);
}

