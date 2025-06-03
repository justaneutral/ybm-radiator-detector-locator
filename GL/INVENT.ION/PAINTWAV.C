// процедуры, относящиеся к маленьким окошкам.

// процедуры, относящиеся к окну "Line in".

#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include "sound.h"
#include "fft.h"

LONG FAR PASCAL __export WaveWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

SMALL_WINDOW  waveWin, mainWin;

void initSmallWindow(SMALL_WINDOW *sm, char *szWinName) {
// INIT DIALOG.
  sm->On      = FALSE;
  sm->hWnd    = NULL;
  sm->szName  = szWinName;
  sm->hBrushG = CreateSolidBrush(    RGB( 128, 128, 128));
  sm->hBrushR = CreateSolidBrush(    RGB( 128, 128,   0));
  sm->hPen    = CreatePen(PS_SOLID,1,RGB( 255, 255, 255));
}  

void clearSmallWindow(SMALL_WINDOW *sm) {
// DESTROY

  sm->On = FALSE;
  sm->hWnd = NULL;
  sm->szName = NULL;
  DeleteObject(sm->hPen);
  DeleteObject(sm->hBrushG);
  DeleteObject(sm->hBrushR);
}  


void graphAmp(HWND hWnd, HDC hdc, RECT rF) {
// термометр амплитуды сигнала

  HBRUSH hOldBrush;
  double am,amm,amx,logAdd,add;
  int rTop;
  
  am  = fabs(waveWin.max);
  amm = fabs(waveWin.min);
  
  if (amm > am) am = amm;
  
  if (am < 1.0) am = 1.0;
  
  add = 128.0;
  
  logAdd = log(add);
  
  if (am < 1.0) am = 1.0;

  am = log(am+add)-logAdd;
  
  if (am < 1.0) am = 1.0;

  amx = log(1024.0*32.0+add)-logAdd;
  
  if (am > amx) am = amx;
  
  amm = (rF.bottom-rF.top)*(1.0-am/amx);
  
  rTop = rF.top+round(amm);
  
  if ( rTop >= rF.bottom) rTop = rF.bottom;

  hOldBrush = SelectObject(hdc, waveWin.hBrushG);
  
  Rectangle(hdc,rF.left,rF.top,rF.right,rF.bottom);

  SelectObject(hdc, waveWin.hBrushR);

  Rectangle(hdc,rF.left,rTop,rF.right,rF.bottom);

  SelectObject(hdc, hOldBrush);
}

void graphR(HWND hWnd, HDC hdc, RECT rF, double *f,
            long nOfSamples, HPEN hPen, HBRUSH hBrush)
// график функции ( для маленьких окошек ).
            
{  
  int    i,j,jb,jf;
  int    x0,y0,x1,dy,l1,l2;
  double minV,maxV,mminV,mmaxV;
  double f3,fp,fs;
  HPEN   hOldPen;     /* old pen handle */

  {
    HBRUSH hOldBrush;

    hOldBrush = SelectObject(hdc, hBrush);
  
    Rectangle(hdc,rF.left,rF.top,rF.right,rF.bottom);

    SelectObject(hdc, hOldBrush);
  }  
  
  if ( (f == NULL) || (nOfSamples < 1) ) 
    return;
  
  x0 = rF.left+5;
  y0 = rF.top+5;
  x1 = rF.right-5;
  dy = (rF.bottom-rF.top)-10;
    
  minV = f[0];
  maxV = f[0]+0.001;
   
  for ( i = 1; i < nOfSamples; i++ ) {
    if ( f[i] < minV ) minV = f[i];
    if ( f[i] > maxV ) maxV = f[i];
  }
  
  waveWin.max = maxV;
  waveWin.min = minV;
  
  f3 = maxV-minV;
   
  hOldPen = SelectObject(hdc, hPen);

  if ( (x1-x0) < nOfSamples ) { // Sample Loop
    if ( (x1-x0) < 1 ) 
      fs = 1.0;
    else  
      fs = nOfSamples / (double)(x1-x0) ;
    fp = fs;
    jf = round(fp);
    i  = x0;
    jb = 0;
   
    while ((i <= x1) && (jf <= nOfSamples)) {
      mmaxV = f[jb];
      mminV = f[jb];
     
      for ( j = jb; j < jf; j++ ) {
        if ( f[j] < mminV ) mminV = f[j];
        if ( f[j] > mmaxV ) mmaxV = f[j];
      }
      
      l1 = round((mminV-minV)/f3*dy);
      MoveTo(hdc,i,y0+dy-l1);
      l2 = round((mmaxV-minV)/f3*dy);
      if (l2 <= l1 )
        l2 = l1+1;
      LineTo(hdc,i,y0+dy-l2);

      i++;
      fp = fp+fs;
      jb = jf;
      jf = round(fp);
    }
  }
  else { // point loop  
    if ( nOfSamples < 2 )
      fs = 1.0;
    else
      fs = ((double)(x1-x0)) / (nOfSamples-1);

    fp = fs;

    l1 = round((f[0]-minV)/f3*dy);
    MoveTo(hdc,x0,y0+dy-l1);
    
    for ( i = 1; i < nOfSamples; i++ ) {
      l2 = round((f[i]-minV)/f3*dy);
      LineTo(hdc,x0+round(fs*i),y0+dy-l2);
    }
  }
  SelectObject(hdc, hOldPen);
}

/* WinMain - Entry point for Reverse.
 */

VOID PASCAL MakeWaveWin(HWND hWndO, HINSTANCE hInst /* HINSTANCE hPrev, LPSTR szCmdLine, int cmdShow */)
{
    WNDCLASS    wc;
    RECT 		rW;
    UINT	    drX = 10,
                drY = 10;

    /* Define and register a window class for the main window.
     */
    if ( waveWin.On == TRUE )
      return;
      
    if ( waveWin.Cl == FALSE ) // !hPrev
    {
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon          = LoadIcon(hInst, waveWin.szName);
        wc.lpszMenuName   = (LPSTR)NULL; // szAppName;
        wc.lpszClassName  = waveWin.szName;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1); //GetStockObject(LTGRAY_BRUSH);
        wc.hInstance      = hInst;
        wc.style          = 0;
        wc.lpfnWndProc    = WaveWndProc;
        wc.cbWndExtra     = 0;
        wc.cbClsExtra     = 0;

        if (!RegisterClass(&wc))
          return;
        waveWin.Cl = TRUE;  
    }

    /* Create and show the main window.
     */
     
    GetWindowRect(hWndO,&rW);
    
    waveWin.hWnd = CreateWindow (waveWin.szName,        // class name
                            waveWin.szName,         // caption
			    			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION |
			    			WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, 
			    			// WS_OVERLAPPEDWINDOW, // style bits
                            rW.left,//CW_USEDEFAULT,     // x position
                            rW.bottom,              // y position
                            (rW.right-rW.left)/2,   //WMAIN_DX, // x size
                            (rW.bottom-rW.top)*2/3,   //WMAIN_DY, // y size
                            (HWND)hWndO,            // parent window
                            (HMENU)NULL,            // use class menu
                            (HANDLE)hInst,          // instance handle
                            (LPSTR)NULL             // no params to pass on
                           );

    GetClientRect(waveWin.hWnd,&rW);
    
    waveWin.L.left = drX+drX+drX/2;
    waveWin.R.left = drX+drX+drX/2;
    
    waveWin.L.right = rW.right-drX;
    waveWin.R.right = rW.right-drX;
    
    waveWin.L.top = drY;
    waveWin.L.bottom = (rW.bottom-drY)/2;
    
    waveWin.R.top = waveWin.L.bottom+drY;
    waveWin.R.bottom = rW.bottom-drY;
    
    // ==
    
    waveWin.LA.left = drX;
    waveWin.RA.left = drX;
    
    waveWin.LA.right = drX*2;
    waveWin.RA.right = drX*2;
    
    waveWin.LA.top = drY;
    waveWin.LA.bottom = (rW.bottom-drY)/2;
    
    waveWin.RA.top = waveWin.L.bottom+drY;
    waveWin.RA.bottom = rW.bottom-drY;

    ShowWindow(waveWin.hWnd,SW_SHOWNORMAL /*cmdShow*/);
    
    waveWin.On = TRUE;
}

VOID drawWave(HDC hdc) {

  HBRUSH hOldBrush; //, hBrush, hBrushA;

  hOldBrush = SelectObject(hdc, waveWin.hBrushG);
  
  Rectangle(hdc,waveWin.L.left,waveWin.L.top,waveWin.L.right,waveWin.L.bottom);
  Rectangle(hdc,waveWin.R.left,waveWin.R.top,waveWin.R.right,waveWin.R.bottom);

  SelectObject(hdc, waveWin.hBrushR);

  Rectangle(hdc,waveWin.LA.left,waveWin.LA.top,waveWin.LA.right,waveWin.LA.bottom);
  Rectangle(hdc,waveWin.RA.left,waveWin.RA.top,waveWin.RA.right,waveWin.RA.bottom);
  
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
    
    waveWin.L.left = drX+drX+drX/2;
    waveWin.R.left = drX+drX+drX/2;
    
    waveWin.L.right = rW.right-drX;
    waveWin.R.right = rW.right-drX;
    
    waveWin.L.top = drY;
    waveWin.L.bottom = (rW.bottom-drY)/2;
    
    waveWin.R.top = waveWin.L.bottom+drY;
    waveWin.R.bottom = rW.bottom-drY;
    
    // ==
    
    waveWin.LA.left = drX;
    waveWin.RA.left = drX;
    
    waveWin.LA.right = drX*2;
    waveWin.RA.right = drX*2;
    
    waveWin.LA.top = drY;
    waveWin.LA.bottom = (rW.bottom-drY)/2;
    
    waveWin.RA.top = waveWin.L.bottom+drY;
    waveWin.RA.bottom = rW.bottom-drY;
}


/* WndProc - Main window procedure function. */

LONG FAR PASCAL __export WaveWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
		  //graphR(hWnd, hdc, waveWin.L, sl, NSAMPLE_CH, waveWin.hPen, waveWin.hBrushG);
		  //graphAmp(hWnd, hdc, waveWin.LA);
		  //graphR(hWnd, hdc, waveWin.R, sr, NSAMPLE_CH, waveWin.hPen, waveWin.hBrushG);
		  //graphAmp(hWnd, hdc, waveWin.RA);
		  ReleaseDC(hWnd,hdc);
         //repaintFreqScale(hWnd,FALSE);
         //freqReport(hWnd);
         break;
      
    case WM_SYSCOMMAND:
        switch (wParam)
        {
        }
        break;
                
    /* Process messages sent by the child window controls.
     */

    case WM_COMMAND:
        switch (wParam)
        {
        case 1:
          hdc = GetDC(hWnd);
		  graphR(hWnd, hdc, waveWin.L, sl, NSAMPLE_CH, waveWin.hPen, waveWin.hBrushG);
		  graphAmp(hWnd, hdc, waveWin.LA);
		  graphR(hWnd, hdc, waveWin.R, sr, NSAMPLE_CH, waveWin.hPen, waveWin.hBrushG);
		  graphAmp(hWnd, hdc, waveWin.RA);
		  ReleaseDC(hWnd,hdc);
          // messageWin(hWnd,"Ok 1");
          return 0;
    	}
        return( 0L );
    }

    return DefWindowProc(hWnd,msg,wParam,lParam);
}


