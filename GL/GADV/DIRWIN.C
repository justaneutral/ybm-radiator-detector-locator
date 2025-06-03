#include "cinc.h"

#include "stdio.h"
#include "string.h"
#include "math.h"

static BOOL DirOn = FALSE; // признак того, что оконный класс уже зарегестрирован.

VOID InitDirWin( LPFUNWIN lpFW, LPCSTR s ) {
  lpFW->On = FALSE;
  //lpFW->Cl = FALSE;
  lpFW->hWnd = NULL;
  lpFW->szName = s;
  //lpFW->d = d;  
}

VOID PASCAL MakeDirWin(HWND hWndO, LPFUNWIN lpFW)
{
    WNDCLASS    wc;
    RECT 		rW;
    UINT	    drX = 10,
                drY = 10;
    LPCSTR		fClassName = "DirWinCls";            

    
    /* Define and register a window class for the main window.
     */
    
    if ( lpFW->On )
      return;
      
    if ( !DirOn ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION); //hInst, waveWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = fClassName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = ghInst;
        wc.style          = 0;
        wc.lpfnWndProc    = DirWindowProc;
        wc.cbWndExtra     = sizeof(LPFUNWIN);
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        DirOn = TRUE;  
    }

    GetWindowRect(hWndO,&rW);
    
    lpFW->hWnd = CreateWindow (fClassName,        // class name
                            lpFW->szName,         // caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			// WS_OVERLAPPEDWINDOW, // style bits
  	    					GetSystemMetrics(SM_CXSCREEN)/2,
							//GetSystemMetrics(SM_CYSCREEN)/5,
                            //rW.left+(rW.right-rW.left)/5,// x position
                            rW.bottom,              // y position
                            //(rW.right-rW.left),   //WMAIN_DX, // x size
                            //(rW.bottom-rW.top)*2,   //WMAIN_DY, // y size
  	    					GetSystemMetrics(SM_CXSCREEN)/2,
							(GetSystemMetrics(SM_CYSCREEN)*2)/5,
                            (HWND)hWndO,            // parent window
                            (HMENU)NULL,            // use class menu
                            (HANDLE)ghInst,          // instance handle
                            (LPSTR)NULL             // no params to pass on
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


VOID drawDir(HDC rDC, RECT *rF) {
  HBRUSH  hOldBrush;
  HPEN  hOldPen;
  short x0,y0,dx,dy,lt;
  DWORD dwColor,dwBColor,dwExt;
  char s[128];

  hOldBrush = SelectObject(rDC, pen.bGray);

  Rectangle(rDC,rF->left,rF->top,rF->right,rF->bottom);

  SelectObject(rDC, hOldBrush);
  
  x0 = (rF->left+rF->right)/2;
  y0 = (rF->top+rF->bottom)/2;
  dx = (rF->right-rF->left-10)/2;
  dy = (rF->bottom-rF->top-10)/2;
  
  if (dx < 1)
    dx = 1;
  if (dy < 1)
    dy = 1;  
  
  //h = min(dy,dx);
  
  hOldPen = SelectObject(rDC, GetStockObject(WHITE_PEN));
  
  MoveTo(rDC,x0-dx,y0);
  LineTo(rDC,x0+dx,y0);

  MoveTo(rDC,x0,y0-dy);
  LineTo(rDC,x0,y0-dy+20);

  MoveTo(rDC,x0,y0+dy-20);
  LineTo(rDC,x0,y0+dy);

  { 
    double r1,dxV,dyV;
    extern double v[4];
    short xV,yV,dRx,dRy,i;
    
    loop ( i, 10 ) {
      r1 = 2*i+1;
      r1 = log(r1+1.0)/log(21.0);  
    
      dRx = round(dx*r1);
      dRy = round(dy*r1);
      
      MoveTo(rDC,x0-dRx,y0+5);
  	  LineTo(rDC,x0-dRx,y0-5);

      MoveTo(rDC,x0+dRx,y0+5);
  	  LineTo(rDC,x0+dRx,y0-5);
    }

    if ( FALSE ) {
      dwColor = SetTextColor(rDC, RGB(255,255,0));
      SelectObject(rDC, pen.pGreen);
    }
    else {  
      dwColor = SetTextColor(rDC, RGB(0,255,255));
      SelectObject(rDC, pen.pHGreen);
    }

    r1 = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])*0.01;

    sprintf(s,"Distance =%5.2f m", r1);
    
    if ( r1 > 20.0 ) 
      r1 = 20.0;
      
    r1 = log(r1+1.0)/log(21.0);  
    
    if (v[0] < 0.0) {
      dxV = -v[0]*0.01;
      if ( dxV > 20.0 )
        dxV = 20.0;
      dxV = -log(1.0+dxV)/log(21.0);
    }
    else {
      dxV = v[0]*0.01;
      if ( dxV > 20.0 )
        dxV = 20.0;
      dxV = log(1.0+dxV)/log(21.0);
    }

    if (v[1] < 0.0) {
      dyV = -v[1]*0.01;
      if ( dyV > 20.0 )
        dyV = 20.0;
      dyV = -log(1.0+dyV)/log(21.0);
    }
    else {
      dyV = v[1]*0.01;
      if ( dyV > 20.0 )
        dyV = 20.0;
      dyV = log(1.0+dyV)/log(21.0);
    }
    
    xV = round(dxV*dx);
    yV = round(dyV*dy);

    if ( TRUE ) {
      SelectObject(rDC, pen.hDashPen);
    }
    else {  
      SelectObject(rDC, pen.hDotPen);
    }

    MoveTo(rDC,x0+xV,y0-yV);
    LineTo(rDC,x0,y0);

    dwBColor = SetBkColor(rDC, RGB(128,128,128));

    //Arc (rDC, x0-dRx, y0-dRy, x0+dRx, y0+dRy, x0-dRx, y0, x0-dRx, y0);

    TextOut (rDC, x0-dx+5, y0-28, "Left", strlen ("Left"));

    dwExt = GetTextExtent (rDC, (LPCSTR)"Right", strlen("Right"));
  
    lt = LOWORD(dwExt);
    
    TextOut (rDC, x0+dx-lt-5, y0-28, "Right", strlen ("Right"));

    dwExt = GetTextExtent (rDC, (LPCSTR)s, strlen(s));
  
    lt = LOWORD(dwExt);
  
    TextOut (rDC, x0+dx-lt-10, y0+dy/2-10, s, strlen(s));

    sprintf(s,"Height =%5.2f m", v[2]*0.01);
  
    TextOut (rDC, x0+dx-lt-10, y0+HIWORD(dwExt)+dy/2-10, s, strlen(s));
    
    sprintf(s,"P =%3.0f", v[3]*0.001);

    lt = HIWORD(dwExt);
  
    TextOut (rDC, x0-dx+10, y0+lt+dy/2-10,s,strlen(s));

  }
  SetTextColor(rDC, dwColor);
  SetBkColor(rDC, dwBColor);

  SelectObject(rDC, hOldPen);
}


VOID drawDirWin(HDC hdc, LPFUNWIN lpFW) {
  drawDir( hdc, &(lpFW->RF) );
}

VOID resizeDirWin( HWND hWnd, HDC hdc, LPFUNWIN lpFW ) {
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


LONG FAR PASCAL __export DirWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  LPFUNWIN lpFW;
  
    lpFW = (LPFUNWIN)GetWindowLong(hWnd,0);
    
    switch (msg)
    {
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
	    drawDirWin(hdc,lpFW);
    	EndPaint(hWnd,&ps);
      }	
	  return 0;    

    case WM_SIZE: 
      if ( lpFW ) {
        hdc = GetDC(hWnd);
        resizeDirWin(hWnd,hdc,lpFW);
  	    drawDirWin(hdc,lpFW);
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
          resizeDirWin(hWnd,hdc,lpFW);
     	  drawDirWin(hdc,lpFW);
     	  ReleaseDC(hWnd,hdc);
     	  lpFW->On = TRUE;
          break;

        
        /*case 100:
          if ( lpFW ) {
            hdc = GetDC(hWnd);
            drawDirWin1(hdc,lpFW);
	        ReleaseDC(hWnd,hdc);
	      }
	      break;
	    */    
        case 100:
          if ( lpFW ) {
            hdc = GetDC(hWnd);
            drawDirWin(hdc,lpFW);
            ReleaseDC(hWnd,hdc);
          }  
          break;
    	}
        return( 0L );
    }

    return DefWindowProc(hWnd,msg,wParam,lParam);
}

