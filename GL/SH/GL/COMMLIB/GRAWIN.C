#include "cinc.h"

//____________ Gra Win (окна с графиками функций) ____________

static BOOL graWinClassOn = FALSE; // признак, что оконный класс
                                   // уже зарегестрирован.

LONG FAR PASCAL __export graWinProc(HWND, UINT, WPARAM, LPARAM);
// оконная функция

/*
   инициализация: ... = {NULL,"Caption",NULL,{0,0,0,0}};
   
   HWND         hWnd;  
   LPCSTR       szName; // caption
   DOUBLEARRAY  d;
   
   после размещение double array прописываем его в окно:
   
   funwin.d = doublearray;
*/   


static VOID calculateGraWinRect( HWND hWnd ) {
// вычисление координат прямоугольника
  RECT 		rW;
  UINT	    drX = 3,
            drY = 3;
  LPFUNWIN  lpFW;          

  GetClientRect(hWnd,&rW);
  lpFW = (LPFUNWIN)GetWindowLong(hWnd,0);
  
  if ( lpFW == NULL )
    return;
    
  lpFW->RF.left = drX;    
  lpFW->RF.right = rW.right-drX;
    
  lpFW->RF.top = drY;
  lpFW->RF.bottom = rW.bottom-drY;
}


static VOID resizeGraWin( HWND hWnd, HDC hdc ) {
// стирание всего, что было нарисовано в окне и
// пересчет координат прямоугольника

  RECT 		rW;
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
    
  calculateGraWinRect(hWnd);  
}


VOID makeGraWin(HWND hWndO, LPFUNWIN lpFW, 
                  short x0, short y0, short dx, short dy)
// создание окна                  
{
    WNDCLASS    wc;
    RECT 		rW;
    LPCSTR		fClassName = "funwincls";            

    if ( lpFW->hWnd != NULL ) {
      errormessage(hWndO, " Err in make Gra Win");
      return;
    }  

    if ( lpFW->d == NULL ) {
      errormessage(hWndO, " Null ptr in Make Win");
      return;
    }  
    
    /* Define and register a window class for the window.
     */
    
    if ( !graWinClassOn ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION); //hInst, waveWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = fClassName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = ghInst;
        wc.style          = 0;
        wc.lpfnWndProc    = graWinProc;
        wc.cbWndExtra     = sizeof(LPFUNWIN);
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc)) {
          message(hWndO,"Register Gra Win Class Error");
          return;
        }  
          
        graWinClassOn = TRUE;  
    }

    GetWindowRect(hWndO,&rW);
    
    lpFW->hWnd = CreateWindow (fClassName,        // class name
                            lpFW->szName,         // caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			// WS_OVERLAPPEDWINDOW, // style bits
                            x0, y0, dx, dy,
                            hWndO,        // parent window
                            NULL,         // use class menu
                            ghInst,       // instance handle
                            (LPSTR)NULL   // no params to pass on
                           );

    if ( lpFW->hWnd == NULL ) {
      message(hWndO,"Create Window Error");
      return;
    }  
    
    SetWindowLong( lpFW->hWnd, 0, (long)lpFW );
    calculateGraWinRect( lpFW->hWnd );
    putGra( lpFW );
    ShowWindow(lpFW->hWnd,SW_SHOWNORMAL);   
}
     

static VOID drawGraWinLP( HDC hdc, LPFUNWIN lpFW ) {
// собственно рисование функции
  HBRUSH hOldBrush;
  
  if ( lpFW->d != NULL ) 
    drawF ( hdc, &(lpFW->RF), lpFW->d );
  else { 
    hOldBrush = SelectObject(hdc, pen.bGray);  
    Rectangle(hdc,lpFW->RF.left,lpFW->RF.top,lpFW->RF.right,lpFW->RF.bottom);
    SelectObject(hdc, hOldBrush);
  }  
}


static VOID drawGraWin(HWND hWnd, HDC hdc) {
// вызов рисования функции
  LPFUNWIN lpFW;

  lpFW = (LPFUNWIN)GetWindowLong(hWnd,0);
  
  if ( lpFW == NULL )
    return;
    
  if ( lpFW->hWnd != hWnd ) {
    errormessage(hWnd,"Gra Win Error");
    return;
  }  
    
  drawGraWinLP( hdc, lpFW );
}


VOID putGra( LPFUNWIN lpFW ) {
// вызов рисования функции
  HDC hdc;

  if ( lpFW == NULL )
    return;
    
  if ( lpFW->hWnd == NULL )
    return;
  
  hdc = GetDC ( lpFW->hWnd );
  drawGraWinLP ( hdc, lpFW );
  ReleaseDC ( lpFW->hWnd, hdc );    
}

      
LONG FAR PASCAL __export graWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
// оконная функция
{
  PAINTSTRUCT ps;
  HDC hdc;
  LPFUNWIN lpFW;
    
  switch (msg) {
  
    case WM_INITDIALOG:
      return TRUE;
      
    case WM_DESTROY:
      lpFW = (LPFUNWIN)GetWindowLong(hWnd,0);
      if ( lpFW )
        lpFW->hWnd = FALSE;
	  return 0;
	    
	case WM_PAINT:
      hdc = BeginPaint(hWnd,&ps);
      drawGraWin(hWnd,hdc);
      EndPaint(hWnd,&ps);
	  return 0;    

    case WM_SIZE: 
      hdc = GetDC(hWnd);
      resizeGraWin(hWnd,hdc);
      drawGraWin(hWnd,hdc);
      ReleaseDC(hWnd,hdc);
      return 0;
    }

    return DefWindowProc(hWnd,msg,wParam,lParam);
}

