#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

#include "resource.h"
#include "sound.h"

//HPEN hDashPen;              /* "---" pen handle   */
//HPEN hDotPen;               /* "..." pen handle   */
//HBRUSH hOldBrush;           /* old brush handle   */
//HBRUSH hRedBrush;           /* red brush handle   */
//HBRUSH hGreenBrush;         /* green brush handle */
//HBRUSH hBlueBrush;          /* blue brush handle  */

void PaintFun(BOOL paintMode, HWND hWnd, double arr[], UINT mArr, UINT id)

{
   HWND  hW;
   HDC   recDC;
   RECT  b;
   UINT  i,l,x0,y0,x1,dy;
   HBRUSH hOldBrush;
   HPEN hOldPen;                     /* old pen handle            */
   PAINTSTRUCT ps;                   /* paint structure           */
  
   
   hW = GetDlgItem(hWnd,id);
   
   GetClientRect(hW, &b);
   
   x0 = b.left+2;
   y0 = b.top+5;
   x1 = b.right;
   dy = (b.bottom-b.top)-15;
   
   if ( paintMode == TRUE ) 
     recDC = BeginPaint(hW,&ps);
   else
     recDC = GetDC(hW);
   hOldBrush = SelectObject(recDC, mainWin.hBrushG);
  
   Rectangle(recDC,b.left,b.top,b.right,b.bottom);

   SelectObject(recDC, hOldBrush);
   
   hOldPen = SelectObject(recDC, mainWin.hPen);
   
   if ( (x1-x0) > mArr) {
   
   for ( i = 0; i < mArr; i++ ) {
     l = round(arr[i]*dy);
     if (l < 1) l = 1;
     
     MoveTo(recDC,i+2,y0+dy-l);
     LineTo(recDC,i+2,y0+dy);
   }
   }
   SelectObject(recDC, hOldPen);
   if ( paintMode == TRUE )
     EndPaint(hW,&ps);
   else  
     ReleaseDC(hW,recDC);
}

void PaintBuff(BOOL paintMode, HWND hWnd, double f[], UINT id)
{  
  HWND  hW;
  HDC   recDC;
  RECT  b;
  UINT  i,x0,y0,x1,dy,j,jb,jf,l1,l2;
  double m1,m2,mm1,mm2;
  double f3,fp,fs;
  HBRUSH hOldBrush;
  HPEN hOldPen;                     /* old pen handle     */
  PAINTSTRUCT ps;                   /* paint structure    */

   
  hW = GetDlgItem(hWnd,id);

  GetClientRect(hW, &b);
       
  x0 = b.left+10;
  y0 = b.top+5;
  x1 = b.right-10;
  dy = (b.bottom-b.top)-15;
   
  if ( paintMode == TRUE ) 
    recDC = BeginPaint(hW,&ps);
  else
    recDC = GetDC(hW);
    
  hOldBrush = SelectObject(recDC, mainWin.hBrushG);
  
  Rectangle(recDC,b.left,b.top,b.right,b.bottom);

  SelectObject(recDC, hOldBrush);

  m1 = f[0];
  m2 = f[0];
   
  for ( i = 0; i < NSAMPLE_IN; i++ ) {
    if ( f[i] < m1 ) m1 = f[i];
    if ( f[i] > m2 ) m2 = f[i];
  }
  
  if (m2 <= m1) f3 = 1.0;
  else f3 = (m2-m1);
   
  fs = NSAMPLE_IN / (x1-x0) ;
  fp = fs;
  jf = round(fp);
  i  = x0;
  jb = 0;
   
  hOldPen = SelectObject(recDC, mainWin.hPen);
  while ((i <= x1) && (jf <= NSAMPLE_IN)) {
    mm1 = f[jb];
    mm2 = f[jb];
     
    for ( j = jb; j < jf; j++ ) {
      if ( f[j] < mm1 ) mm1 = f[j];
      if ( f[j] > mm2 ) mm2 = f[j];
    }
     
    l1 = round((mm1-m1)/f3*dy);
    MoveTo(recDC,i,y0+l1);
    l2 = round((mm2-m1)/f3*dy);
    if (l2 <= l1 )
      l2 = l1+1;
    LineTo(recDC,i,y0+l2);

    i++;
    fp = fp+fs;
    jb = jf;
    jf = round(fp);
  }
  SelectObject(recDC, hOldPen);
  if ( paintMode == TRUE )
    EndPaint(hW,&ps);
  else  
    ReleaseDC(hW,recDC);
}

VOID messageWin(HWND hWnd, char *s) {
  MessageBox(hWnd,s," ", MB_OK | MB_ICONEXCLAMATION );
}

int printW(HWND hDlg, UINT ne, char *fmt, ...)
{
    va_list marker;
    char    buffer[256];
    int     size = 256, written = -1;

    va_start( marker, fmt );
    written = _vsnprintf( &(buffer[0]), size, fmt, marker );
    if ( written != -1) 
      SendDlgItemMessage(hDlg,ne,WM_SETTEXT,0,(LPARAM)(LPSTR)buffer);

    va_end( marker );
    return written;
}


