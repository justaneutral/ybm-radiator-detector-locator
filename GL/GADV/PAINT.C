#include "cinc.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
 

struct penTag  pen = {NULL,NULL,NULL};
struct DoubleVTag { double max,min; } DoubleV;
  

short round(double f) {
  return (short)(f+0.5);
}

BOOL ColorsOn( BOOL St ) { 
  /* INIT DIALOG. */
  
  if ( St ) { /* CreateColors */
  
    pen.bGray    = CreateSolidBrush(    RGB( 128, 128, 128));
    pen.hGrayPen = CreatePen(PS_SOLID,1,RGB( 128, 128, 128));
    pen.bGreen   = CreateSolidBrush(    RGB( 128, 128,   0));
    pen.pGreen   = CreatePen(PS_SOLID,1,RGB( 255, 255,   0));
    pen.hDashPen = CreatePen(PS_DASH, 1,RGB( 255, 255,   0));  
    /* ... : CreatePen(2, 1, RGB(0, 0, 0)); */
    pen.hDotPen  = CreatePen(PS_DASH, 1,RGB(   0, 255, 255));
    pen.pHGreen  = CreatePen(PS_SOLID,1,RGB(   0, 255, 255));
  } 
  else { /* DestroyColors */
  
    DeleteObject(pen.bGray);
    DeleteObject(pen.bGreen);
    DeleteObject(pen.pGreen);
    DeleteObject(pen.hDashPen);
    DeleteObject(pen.hDotPen);
  }
  return TRUE;
} 


VOID drawAmp(HDC rdc, RECT *rF, DOUBLEARRAY d) {
  HBRUSH  hOldBrush;
  double  am,amm,amx,logAdd,add;
  short   rTop;

  /* термометр амплитуды сигнала */
  
  if ( d == NULL ) return;

  am  = fabs(DoubleV.max);
  amm = fabs(DoubleV.min);

  if ( amm > am ) am = amm;

  if ( am < 1.0 ) am = 1.0;

  add = 128.0;

  logAdd = log(add);

  if ( am < 1.0 ) am = 1.0;

  am = log(am+add)-logAdd;

  if ( am < 1.0 ) am = 1.0;

  amx = log(1024.0*32.0+add)-logAdd;

  if ( am > amx ) am = amx;

  amm = (rF->bottom-rF->top)*(1.0-am/amx);

  rTop = rF->top+round(amm);

  if ( rTop >= rF->bottom ) rTop = rF->bottom;

  hOldBrush = SelectObject(rdc, pen.bGray);

  Rectangle(rdc,rF->left,rF->top,rF->right,rF->bottom);

  SelectObject(rdc, pen.bGreen);

  Rectangle(rdc,rF->left,rTop,rF->right,rF->bottom);

  SelectObject(rdc, hOldBrush);
} /* graphAmp */


VOID drawF(HDC rDC, RECT *rF, DOUBLEARRAY d) {
HBRUSH  hOldBrush;
HPEN  hOldPen;
short i,j,jb,jf,x0,y0,x1,dy,l1,l2,m;
double  minV,maxV,mminV,mmaxV,f3,fp,fs;


  hOldBrush = SelectObject(rDC, pen.bGray);

  Rectangle(rDC,rF->left,rF->top,rF->right,rF->bottom);

  SelectObject(rDC, hOldBrush);

  if ( d == NULL )
    return;
    
  m = GetMemDC(d)->m;
  
  if ( m == 0 )
    return;

  x0 = rF->left+5;
  y0 = rF->top+5;
  x1 = rF->right-5;
  dy = (rF->bottom-rF->top)-10;

  minV = d[0];
  maxV = d[0]+0.001;

  for ( i = 1; i < m; i++ ) {
    if ( d[i] < minV ) minV = d[i];
    if ( d[i] > maxV ) maxV = d[i];
  }

  DoubleV.max = maxV;
  DoubleV.min = minV;
  
  f3 = maxV-minV;

  hOldPen = SelectObject(rDC, GetStockObject(WHITE_PEN));

  if ( (x1-x0) < m ) { /* // Sample Loop  */
    if ( (x1-x0) < 1 )
      fs = 1.0;
    else
      fs = ((double)m) / (x1-x0);

    fp = fs;
    jf = round(fp);
    i  = x0;
    jb = 0;

    while ( (i <= x1) and (jf < m) ) {
      mmaxV = d[jb];
      mminV = d[jb];

      for ( j = jb; j < jf; j++ ) {
        if ( d[j] < mminV ) mminV = d[j];
        if ( d[j] > mmaxV ) mmaxV = d[j];
      }

      l1 = round((mminV-minV)/f3*dy);
      MoveTo(rDC,i,y0+dy-l1);
      l2 = round((mmaxV-minV)/f3*dy);
      if ( l2 <= l1 )
        l2 = l1+1;
      LineTo(rDC,i,y0+dy-l2);

      i++;
      fp = fp+fs;
      jb = jf;
      jf = round(fp);
    }
  }
  else {/* // point loop */
    if ( m < 2 )
      fs = 1.0;
    else
      fs = ((double)(x1-x0))/(m-1);

    fp = fs;

    l1 = round((d[0]-minV)/f3*dy);
    MoveTo(rDC,x0,y0+dy-l1);

    for ( i = 1; i < m; i++ ) {
      l2 = round((d[i]-minV)/f3*dy);
      LineTo(rDC,x0+round(fs*i),y0+dy-l2);
    }
  }
  SelectObject(rDC, hOldPen);
}


int printW(HWND hDlg, UINT ne, char *fmt, ...)
{
    va_list marker;
    char    buffer[256];
    int     size = 256, written = -1;

    va_start( marker, fmt );
    written = _vsnprintf( buffer, size, fmt, marker );
    if ( written != -1) 
      SendDlgItemMessage(hDlg,ne,WM_SETTEXT,0,(LPARAM)(LPCSTR)buffer);

    va_end( marker );
    return written;
}


