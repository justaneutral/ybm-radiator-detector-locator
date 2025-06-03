#include "cinc.h"
#include "scan.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

static BOOL PowerScaleOn = FALSE; // признак того, что оконный класс уже зарегестрирован.

static struct MarkerPowerTag { BOOL On;
                   long Pos;
                 } MarkerPowerStr = { FALSE, 0 };  

FUNWIN fwPowerScale;


VOID InitPowerScaleWin( LPCSTR s ) {
  fwPowerScale.On = FALSE;
  fwPowerScale.hWnd = NULL;
  fwPowerScale.szName = s;
}


VOID PASCAL MakePowerScaleWin(HWND hWndO)
{
    WNDCLASS    wc;
    RECT 		rW;
    UINT	    drX = 10,
                drY = 10;
    LPCSTR		fClassName = "PowerCls";            
    
    /* Define and register a window class for the main window.
     */
    
    if ( fwPowerScale.On )
      return;
      
    if ( !PowerScaleOn ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION); //hInst, waveWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = fClassName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = ghInst;
        wc.style          = 0;
        wc.lpfnWndProc    = PowerScaleWindowProc;
        wc.cbWndExtra     = sizeof(LPFUNWIN);
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        PowerScaleOn = TRUE;  
    }

    GetWindowRect(hWndO,&rW);
    
    fwPowerScale.hWnd = CreateWindow (fClassName,        // class name
                            fwPowerScale.szName,         // caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			// WS_OVERLAPPEDWINDOW, // style bits
                            rW.left, //+(rW.right-rW.left)/2,// x position
                            rW.bottom,              // y position
                            (rW.right-rW.left),   //WMAIN_DX, // x size
                            (rW.bottom-rW.top)*3/2,   //WMAIN_DY, // y size
                            (HWND)hWndO,            // parent window
                            (HMENU)NULL,            // use class menu
                            (HANDLE)ghInst,          // instance handle
                            (LPSTR)NULL             // no params to pass on
                           );

    // SendMessage(fwPowerScale.hWnd, WM_COMMAND,101,(long)lpFW);

    GetClientRect(fwPowerScale.hWnd,&rW);
    
    fwPowerScale.On = TRUE;
    
    fwPowerScale.RF.left = drX;    
    fwPowerScale.RF.right = rW.right-drX;
    
    fwPowerScale.RF.top = drY;
    fwPowerScale.RF.bottom = rW.bottom-drY;
    
    ShowWindow(fwPowerScale.hWnd,SW_SHOWNORMAL);   
}


//void PutMarker(BOOL paintMode, HWND hWnd, long pM, long nOfSamples, UINT id)

static VOID DrawPowerMarkerLine(HDC rDC) {
  //HBRUSH  hOldBrush;
  HPEN  hOldPen;
  long i;
  short x0,y0,x1,dy,xM;
  int   oldROP;
  double fs;
  RECT rF;

  rF = fwPowerScale.RF;
     
  x0 = rF.left+5;
  y0 = rF.top+5;
  x1 = rF.right-5;
  dy = (rF.bottom-rF.top)-10;
      
  i = MarkerPowerStr.Pos;

  if ( 
       ( CurTab.nMax < 1 ) or 
       ( i >= CurTab.nMax ) or
       ( i < 0 ) or
       ( CurTab.LastFreq < 1 ) or 
       ( (x1-x0) < 1 ) or 
       ( CurTab.LastFreq < CurTab.FirstFreq ) or
       ( CurTab.FreqMax == NULL) or
       ( dy < 1 )
     )
    return;

  if ( CurTab.LastFreq <= CurTab.FirstFreq ) 
    fs = 0.0;
  else  
    fs = (x1-x0) / ( (double)(CurTab.LastFreq-CurTab.FirstFreq) );

  xM = x0+round (fs*(CurTab.FreqMax[i].fMid.f-CurTab.FirstFreq) );
    
  hOldPen = SelectObject(rDC, GetStockObject(WHITE_PEN));
  //SelectObject( rDC, GetStockObject (WHITE_PEN) );
  oldROP = SetROP2(rDC,R2_NOT);
  MoveTo(rDC,xM,y0+dy);
  LineTo(rDC,xM,y0);
  SetROP2(rDC,oldROP);
  SelectObject(rDC, hOldPen);
}


VOID PutPowerMarker(long NewPos) {
  HDC rDC;

  rDC = GetDC(fwPowerScale.hWnd);

  if ( MarkerPowerStr.On ) {
    DrawPowerMarkerLine(rDC);
    MarkerPowerStr.On = FALSE;
  }  
  
  if ( NewPos >= 0 ) {
    MarkerPowerStr.Pos = NewPos;
    DrawPowerMarkerLine(rDC);
    MarkerPowerStr.On = TRUE;
  }

  ReleaseDC(fwPowerScale.hWnd,rDC);
}


VOID ClearPowerScale(HDC rDC) {
  HBRUSH  hOldBrush;
  //HPEN  hOldPen;
  short x0,y0,x1,dy;
  RECT rF;

  rF = fwPowerScale.RF;
     
  x0 = rF.left+5;
  y0 = rF.top+5;
  x1 = rF.right-5;
  dy = (rF.bottom-rF.top)-10;

  hOldBrush = SelectObject(rDC, pen.bGray);
  Rectangle(rDC,rF.left,rF.top,rF.right,rF.bottom);
  SelectObject(rDC, hOldBrush);
  
  if ( MarkerPowerStr.On ) {
    DrawPowerMarkerLine(rDC);
  }  
}


// рисование максимумов c m1-ого по m2-ой.


BOOL drawPowerMax(HDC rDC, long m1, long m2) {
  HBRUSH  hOldBrush;
  HPEN  hOldPen;
  long i,im;
  short x0,y0,x1,dy,xM,yM;
  double  fAmp,fs;
  RECT rF;

  rF = fwPowerScale.RF;
  
  x0 = rF.left+5;
  y0 = rF.top+5;
  x1 = rF.right-5;
  dy = (rF.bottom-rF.top)-10;
    
  if ( ( m1 >= m2 ) or 
       ( CurTab.nMax < 1 ) or 
       ( m2 < 1 ) or 
       ( m2 > CurTab.nMax ) or
       ( m1 < 0 ) or
       ( CurTab.LastFreq < 1 ) or 
       ( (x1-x0) < 1 ) or 
       ( CurTab.LastFreq < CurTab.FirstFreq ) or
       ( CurTab.FreqMax == NULL) )
    return FALSE;

  if ( dy < 1 )
    return FALSE;

  fAmp = 0.1;
  im = 0;
  
  loop (i, CurTab.nMax) 
    if ( CurTab.FreqMax[i].power > fAmp ) {
      fAmp = CurTab.FreqMax[i].power;
      im = i;
    }  
    
  if ( CurTab.LastFreq <= CurTab.FirstFreq ) 
    fs = 0.0;
  else  
    fs = (x1-x0)/((double)(CurTab.LastFreq-CurTab.FirstFreq));

  {
    short xl,xr;
    
    xl = x0+round(fs*(CurTab.FreqMax[m1].fMid.f-CurTab.FirstFreq) );
    xr = x0+round(fs*(CurTab.FreqMax[m2-1].fMid.f-CurTab.FirstFreq));

    hOldBrush = SelectObject ( rDC, pen.bGray );
    hOldPen = SelectObject( rDC, pen.hGrayPen );
    //Rectangle(rDC,rF.left,rF.top,rF.right,rF.bottom);
    //Rectangle(rDC,xl,y0,xr,yd);
    Rectangle(rDC,xl,rF.top,xr,rF.bottom);

    SelectObject(rDC, hOldBrush);
  }

  SelectObject( rDC, GetStockObject (WHITE_PEN) );


  for ( i = m1; i < m2; i++ ) {
    yM = y0+dy;
    xM = x0+round(fs*(CurTab.FreqMax[i].fMid.f-CurTab.FirstFreq));    
    MoveTo (rDC,xM,yM);
    yM = y0+dy-round(CurTab.FreqMax[i].power/fAmp*dy );
    //xM = x0+round(fs*(FreqMax[i].fMid.f-FirstFreq) );
    LineTo (rDC,xM,yM);
  }
  SelectObject(rDC, hOldPen);

  if ( MarkerPowerStr.On ) 
    if ( ( MarkerPowerStr.Pos >= m1 ) and ( MarkerPowerStr.Pos >= m1 ) ) 
      DrawPowerMarkerLine(rDC);

  if ( ( im >= m1 ) and ( im < m2 ) )
    return TRUE;
  return FALSE;  
}


VOID reDrawPowerScale( void ) {
  HDC rDC;
  
  if ( fwPowerScale.On ) {
    rDC = GetDC(fwPowerScale.hWnd);
    ClearPowerScale(rDC);
    drawPowerMax(rDC, 0, CurTab.nMax);
    ReleaseDC(fwPowerScale.hWnd,rDC);
  }  
}    


VOID resizePowerScaleWin( HWND hWnd, HDC hdc ) {
  RECT 		rW;
  UINT	    drX = 10,
            drY = 10;
            
  HBRUSH 	hBrushW,hOldBrush;
  HPEN   	hPenW,hOldPen;
    

  hPenW = GetStockObject(WHITE_PEN);
  hBrushW = GetStockObject(WHITE_BRUSH);

  GetClientRect(hWnd, &rW);
    
  hOldPen = SelectObject(hdc, hPenW);
  hOldBrush = SelectObject(hdc, hBrushW);
    
  Rectangle(hdc, rW.left, rW.top, rW.right, rW.bottom);
    
  SelectObject(hdc, hOldPen);
  SelectObject(hdc, hOldBrush);
    
  fwPowerScale.RF.left = drX;    
  fwPowerScale.RF.right = rW.right-drX;
    
  fwPowerScale.RF.top = drY;
  fwPowerScale.RF.bottom = rW.bottom-drY;
  
  ClearPowerScale(hdc);
  
  drawPowerMax(hdc, 0, CurTab.nMax);
}


LONG FAR PASCAL __export PowerScaleWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  
    switch (msg)
    {
    case WM_INITDIALOG:
      return TRUE;
      
    case WM_DESTROY:
      fwPowerScale.On = FALSE;
	    //PostQuitMessage(0);
	  return 0;
	    
	case WM_PAINT:
      hdc = BeginPaint(hWnd,&ps);
      ClearPowerScale(hdc);
      drawPowerMax(hdc, 0, CurTab.nMax);
      EndPaint(hWnd,&ps);
	  return 0;    

    case WM_SIZE: 
      hdc = GetDC(hWnd);
      resizePowerScaleWin(hWnd,hdc);
      ReleaseDC(hWnd,hdc);
      break;
    }
    return DefWindowProc(hWnd,msg,wParam,lParam);
}

