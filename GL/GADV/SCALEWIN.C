#include "cinc.h"
#include "scan.h"
#include "stdio.h"
#include "string.h"
#include "math.h"

static BOOL ScaleOn = FALSE; // признак того, что оконный класс уже зарегестрирован.

struct MarkerTag { BOOL On;
                   long Pos;
                 } MarkerStr = { FALSE, 0 };  

FUNWIN fwScale;


VOID InitScaleWin( LPCSTR s ) {
  fwScale.On = FALSE;
  //fwScale.Cl = FALSE;
  fwScale.hWnd = NULL;
  fwScale.szName = s;
  //fwScale.d = d;  
}


VOID PASCAL MakeScaleWin(HWND hWndO)
{
    WNDCLASS    wc;
    RECT 		rW;
    UINT	    drX = 10,
                drY = 10;
    LPCSTR		fClassName = "ScaWinCls";            
    
    /* Define and register a window class for the main window.
     */
    
    if ( fwScale.On )
      return;
      
    if ( !ScaleOn ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(NULL,IDI_APPLICATION); //hInst, waveWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = fClassName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = ghInst;
        wc.style          = 0;
        wc.lpfnWndProc    = ScaleWindowProc;
        wc.cbWndExtra     = sizeof(LPFUNWIN);
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        ScaleOn = TRUE;  
    }

    GetWindowRect(hWndO,&rW);
    
    fwScale.hWnd = CreateWindow (fClassName,        // class name
                            fwScale.szName,         // caption
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

    // SendMessage(fwScale.hWnd, WM_COMMAND,101,(long)lpFW);

    GetClientRect(fwScale.hWnd,&rW);
    
    fwScale.On = TRUE;
    
    fwScale.RF.left = drX;    
    fwScale.RF.right = rW.right-drX;
    
    fwScale.RF.top = drY;
    fwScale.RF.bottom = rW.bottom-drY;
    
    //fwScale.On = TRUE;
 
    ShowWindow(fwScale.hWnd,SW_SHOWNORMAL);   
}


//void PutMarker(BOOL paintMode, HWND hWnd, long pM, long nOfSamples, UINT id)

VOID DrawMarkerLine(HDC rDC) {
  //HBRUSH  hOldBrush;
  HPEN  hOldPen;
  long i;
  short x0,y0,x1,dy,xM;
  int   oldROP;
  double fs;
  RECT rF;

  rF = fwScale.RF;
     
  x0 = rF.left+5;
  y0 = rF.top+5;
  x1 = rF.right-5;
  dy = (rF.bottom-rF.top)-10;
      
  i = MarkerStr.Pos;

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
  oldROP = SetROP2(rDC,R2_NOT);
  MoveTo(rDC,xM,y0+dy);
  LineTo(rDC,xM,y0);
  SetROP2(rDC,oldROP);
  SelectObject(rDC, hOldPen);
}


VOID PutMarker(HDC rDC, long NewPos) {

  if ( MarkerStr.On ) {
    DrawMarkerLine(rDC);
    MarkerStr.On = FALSE;
  }  
  
  if ( NewPos >= 0 ) {
    MarkerStr.Pos = NewPos;
    DrawMarkerLine(rDC);
    MarkerStr.On = TRUE;
  }
}


VOID ClearFreqScale(HDC rDC) {
  HBRUSH  hOldBrush;
  //HPEN  hOldPen;
  short x0,y0,x1,dy;
  RECT rF;

  rF = fwScale.RF;
     
  x0 = rF.left+5;
  y0 = rF.top+5;
  x1 = rF.right-5;
  dy = (rF.bottom-rF.top)-10;

  hOldBrush = SelectObject(rDC, pen.bGray);
  Rectangle(rDC,rF.left,rF.top,rF.right,rF.bottom);
  SelectObject(rDC, hOldBrush);
  
  if ( MarkerStr.On ) {
    MarkerStr.On = FALSE;
    PutMarker( rDC, MarkerStr.Pos );
  }  
}


// рисование максимумов c m1-ого по m2-ой.

VOID drawFreqMax(HDC rDC, long m1, long m2) {
  //HBRUSH  hOldBrush;
  HPEN  hOldPen;
  long i;
  short x0,y0,x1,dy,xM,yM;
  double  fAmp,fs;
  RECT rF;

  rF = fwScale.RF;
     
  x0 = rF.left+5;
  y0 = rF.top+5;
  x1 = rF.right-5;
  dy = (rF.bottom-rF.top)-10;
    
  if ( ( m1 > m2 ) or 
       ( CurTab.nMax < 1 ) or 
       ( m2 < 1 ) or
       ( CurTab.LastFreq < 1 ) or 
       ( (x1-x0) < 1 ) or 
       ( CurTab.LastFreq < CurTab.FirstFreq ) or
       ( CurTab.FreqMax == NULL) )
    return;

  if ( dy < 1 )
    return;

  fAmp = 17;
    
  hOldPen = SelectObject(rDC, GetStockObject(WHITE_PEN));

  if ( CurTab.LastFreq <= CurTab.FirstFreq ) 
    fs = 0.0;
  else  
    fs = (x1-x0) / ( (double)(CurTab.LastFreq-CurTab.FirstFreq) );

  for ( i = m1; i < m2; i++ ) {

    yM = y0+dy-round ( CurTab.FreqMax[i].fMin.v/fAmp*dy );
    xM = x0+round ( fs*(CurTab.FreqMax[i].fMin.f-CurTab.FirstFreq) );
    
    MoveTo(rDC,xM,yM);

    yM = y0+dy-round ( CurTab.FreqMax[i].fMid.v/fAmp*dy );
    xM = x0+round ( fs*(CurTab.FreqMax[i].fMid.f-CurTab.FirstFreq) );

    LineTo(rDC,xM,yM);

    yM = y0+dy-round ( CurTab.FreqMax[i].fMax.v/fAmp*dy );
    xM = x0+round ( fs*(CurTab.FreqMax[i].fMax.f-CurTab.FirstFreq) );

    LineTo(rDC,xM,yM);
  }
  SelectObject(rDC, hOldPen);

  if ( MarkerStr.On ) 
    if ( ( MarkerStr.Pos >= m1 ) and ( MarkerStr.Pos >= m1 ) ) {
      MarkerStr.On = FALSE;
      PutMarker( rDC, MarkerStr.Pos );
    }  

}



VOID reDrawScale( void ) {
  HDC rDC;
  
  if ( fwScale.On ) {
    rDC = GetDC(fwScale.hWnd);
    ClearFreqScale(rDC);
    drawFreqMax(rDC, 0, CurTab.nMax);
    ReleaseDC(fwScale.hWnd,rDC);
  }  
}    


VOID resizeScaleWin( HWND hWnd, HDC hdc ) {
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
    
  fwScale.RF.left = drX;    
  fwScale.RF.right = rW.right-drX;
    
  fwScale.RF.top = drY;
  fwScale.RF.bottom = rW.bottom-drY;
  
  ClearFreqScale(hdc);
  
  drawFreqMax(hdc, 0, CurTab.nMax);
}


LONG FAR PASCAL __export ScaleWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps;
  HDC hdc;
  
    switch (msg)
    {
    case WM_INITDIALOG:
      return TRUE;
      
    case WM_DESTROY:
      fwScale.On = FALSE;
	    //PostQuitMessage(0);
	  return 0;
	    
	case WM_PAINT:
      hdc = BeginPaint(hWnd,&ps);
      ClearFreqScale(hdc);
      drawFreqMax(hdc, 0, CurTab.nMax);
      EndPaint(hWnd,&ps);
	  return 0;    

    case WM_SIZE: 
      hdc = GetDC(hWnd);
      resizeScaleWin(hWnd,hdc);
      ReleaseDC(hWnd,hdc);
      break;
    }
    return DefWindowProc(hWnd,msg,wParam,lParam);
}

