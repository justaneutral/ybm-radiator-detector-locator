#include <windows.h>
#include <mmsystem.h>

#include "resource.h"
#include "scnpaint.h"
#include "sound.h"


short 		round(double f);

struct scanScaleTag scS;
  
int scanCount = 0;

HGLOBAL    hAFM = NULL;    // array of freq max's.
arrFreqMax afm = NULL;     // -"-

UINT	initFreqData(long f1, long f2, long fStep) { // prepare for drawing
  long nV;
  
  if ( scS.hV != NULL )  
    return FALSE;
    
  if ( scS.pV != NULL )
    return FALSE;

  if (fStep <= 0)
    return FALSE;
    
  if (f2 < f1)
    return FALSE;
    
  nV = ((f2-f1) / fStep)+1;
  
  if ( nV > 32000 )
    return FALSE;
  
  scS.f1 = f1;
  scS.f2 = f2;
  scS.fStep = fStep;
        
  scS.nV = (UINT)nV;
  scS.nxtV = 0;
  scS.hV = GlobalAlloc(GPTR,nV);
  if ( scS.hV == NULL )
    return FALSE;
  
  scS.pV = GlobalLock(scS.hV);
  
  if ( scS.pV == NULL ) {
    GlobalFree(scS.hV);
    scS.hV = NULL;
    scS.pV = NULL;
    return FALSE;
  }  

  return TRUE;
}

UINT	clearFreqPaint( void ) { // unprepare
  //UINT i;

  if ( scS.hV == NULL) return FALSE;

  if ( scS.pV == NULL) return FALSE;
  
  if (GlobalUnlock(scS.hV) != 0) return FALSE;
  
  if ( GlobalFree(scS.hV) != NULL ) return TRUE;

  scS.hV = NULL;

  scS.pV = NULL;

  return TRUE;
}

UINT	clearAFM( void ) { // unprepare
  //UINT i;

  if ( hAFM == NULL) return FALSE;

  if ( afm == NULL) return FALSE;
  
  if (GlobalUnlock(hAFM) != 0) return FALSE;
  
  if ( GlobalFree(hAFM) != NULL ) return TRUE;

  hAFM = NULL;

  afm = NULL;

  return TRUE;
}


void graphic(BOOL paintMode, HWND hWnd, char *f, long nOfSamples, UINT id)
{  
  HWND   hW;
  HDC    recDC;
  RECT   b;
  int    i,j,jb,jf;
  int    x0,y0,x1,dy,l1,l2;
  double minV,maxV,mminV,mmaxV;
  double f3,fp,fs;
  HBRUSH hOldBrush;
  HPEN   hOldPen;         /* old pen handle            */
  PAINTSTRUCT ps;         /* paint structure           */
  
  hW = GetDlgItem(hWnd,id);

  GetClientRect(hW, &b);

  if ( paintMode == TRUE ) 
    recDC = BeginPaint(hW,&ps);
  else
    recDC = GetDC(hW);

  hOldBrush = SelectObject(recDC, mainWin.hBrushG);
  
  Rectangle(recDC,b.left,b.top,b.right,b.bottom);

  SelectObject(recDC, hOldBrush);

  if ( f != NULL ) {
  
  x0 = b.left+5;
  y0 = b.top+5;
  x1 = b.right-5;
  dy = (b.bottom-b.top)-10;
   

  minV = 0;
  maxV = 16;
   
  for ( i = 0; i < nOfSamples; i++ ) {
    if (( f[i] < minV ) || ( f[i] > maxV )) 
      MessageBox( hWnd, " f value error ", " ",
                   MB_ICONEXCLAMATION ) ;
  }
  
  if (maxV <= minV) f3 = 1.0;
  else f3 = (maxV-minV);
   
  hOldPen = SelectObject(recDC, mainWin.hPen);

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
      //mminV = f[jb];
      mmaxV = f[jb];
     
      for ( j = jb; j < jf; j++ ) {
        //if ( f[j] < mminV ) mminV = f[j];
        if ( f[j] > mmaxV ) mmaxV = f[j];
      }
      
      mminV = 0.0;
      
      l1 = round((mminV-minV)/f3*dy);
      MoveTo(recDC,i,y0+dy-l1);
      l2 = round((mmaxV-minV)/f3*dy);
      if (l2 <= l1 )
        l2 = l1+1;
      LineTo(recDC,i,y0+dy-l2);

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
    MoveTo(recDC,x0,y0+dy-l1);
    
    for ( i = 1; i < nOfSamples; i++ ) {
      l2 = round((f[i]-minV)/f3*dy);
      LineTo(recDC,x0+round(fs*i),y0+dy-l2);
    }
  }
  SelectObject(recDC, hOldPen);

  }

  if ( paintMode == TRUE )
    EndPaint(hW,&ps);
  else  
    ReleaseDC(hW,recDC);
}

void marker(BOOL paintMode, HWND hWnd, long pM, long nOfSamples, UINT id)
{  
  HWND   hW;
  HDC    recDC;
  RECT   b;
  int    i,oldROP;
  int    x0,y0,x1,dy;
  //HPEN   hOldPen;         /* old pen handle            */
  PAINTSTRUCT ps;         /* paint structure           */
  
  hW = GetDlgItem(hWnd,id);

  GetClientRect(hW, &b);

  if ( paintMode == TRUE ) 
    recDC = BeginPaint(hW,&ps);
  else
    recDC = GetDC(hW);

  x0 = b.left+5;
  y0 = b.top+5;
  x1 = b.right-5;
  dy = (b.bottom-b.top)-10;
   
  //hOldPen = SelectObject(recDC, mainWin.hPen);

    i = round((((double)(x1-x0))*pM)/nOfSamples)+x0;
    
    oldROP = SetROP2(recDC,R2_NOT);
    MoveTo(recDC,i,y0);
    LineTo(recDC,i,y0+dy);
    //SelectObject(recDC, hOldPen);
    SetROP2(recDC,oldROP);

  if ( paintMode == TRUE )
    EndPaint(hW,&ps);
  else  
    ReleaseDC(hW,recDC);
}

UINT	repaintFreqScale(HWND hWnd, UINT mode) { // Paint / ReDraw

  graphic(mode,hWnd,scS.pV,scS.nV,IDC_WIN_PICTURE);

  return TRUE;
}


UINT	addScanSample(HWND hWnd, char v) { // one reading
  long cF;
  
  if (scS.nxtV >= scS.nV)
    return FALSE;
    
  scS.pV[scS.nxtV] = v; 
    
  scS.nxtV++;
  
  cF = scS.f1+scS.fStep*scS.nxtV;
  
  scanCount++;
  
  if (scanCount > 10)
    repaintFreqScale(hWnd,FALSE);
  
  if (cF > scS.f2) return FALSE;
  
  return TRUE;
}


VOID getMaxFreq(HWND hWnd) {
  UINT i, pm;
  char tV;
  
  enum {up,down,eq} qn,qc;
  
  if (scS.nxtV < 2) {
    MessageBox( hWnd, "max save error", " ",MB_ICONEXCLAMATION ) ;  
    return;
  }
  
  if ( ( scS.pV == NULL ) || (scS.hV == NULL ) ) {
    MessageBox( hWnd, "max save error", " ",MB_ICONEXCLAMATION ) ;  
    return;
  }
  
  if (scS.pV[0] < scS.pV[1]) qc = up;
  else qc = down;
  
  pm = 0;
  
  for ( i = 1; i < scS.nxtV; i++ ) {
    tV = scS.pV[i-1]; 
    if (tV < scS.pV[i]) qn = up;
    else if (tV == scS.pV[i]) qn = eq;
    else qn = down;
    
    scS.pV[i-1] = 0;
    if ((qc == up) && (qn == eq)) {
      pm = i-1;
    }
    else if ((qc == up) && (qn == up)) {
      pm = i;
    }  
    else if ((qc == up) && (qn == down)) {
      qc = down;
      scS.pV[pm+((i-pm) / 2)] = tV;
    } 
    else if ((qc == down) && (qn == up)) {
      qc = up;
      pm = i;
    }    
  }
  
  i = scS.nxtV;
  tV = scS.pV[i-1];    
  scS.pV[i-1] = 0;
  
  if (qc == up) 
      scS.pV[pm+((i-pm) / 2)] = tV;
}


VOID SaveMaxFreq(HWND hWnd) {
  HFILE hfReadFile;
  UINT i,nm;
  struct freqMaxTag curMax; 
  
  if (scS.nxtV < 2) {
    MessageBox( hWnd, "max save error", " ",MB_ICONEXCLAMATION ) ;  
    return;
  }
  
  if ( ( scS.pV == NULL ) || (scS.hV == NULL ) ) {
    MessageBox( hWnd, "max save error", " ",MB_ICONEXCLAMATION ) ;  
    return;
  }

  getMaxFreq(hWnd);
  
  nm = 0;
  
  for (i = 0; i < scS.nxtV; i++)
    if (scS.pV[i] > 0) nm++;
    
  scS.nMax = nm;
  
  hfReadFile = _lcreat("freqmax.dat", 0);

  if ( hfReadFile == HFILE_ERROR )
    MessageBox( hWnd, "file open error (w)", " ",MB_ICONEXCLAMATION ) ;
    
  if ( _lwrite(hfReadFile,&(scS),sizeof(scS)) != sizeof(scS) )
      MessageBox( hWnd, " file write error 2", " ",MB_ICONEXCLAMATION ) ;

  for ( i = 0; i < scS.nxtV; i++ )
    if (scS.pV[i] > 0) {
      curMax.ac = i;
      curMax.v = scS.pV[i];
      curMax.pr = scS.pV[i];
      curMax.f = scS.f1+i*scS.fStep;
      if ( _lwrite(hfReadFile,&(curMax),sizeof(curMax)) != sizeof(curMax) ) {
        MessageBox( hWnd, " file write error 3", " ",MB_ICONEXCLAMATION ) ;
        _lclose(hfReadFile);
        return;
      }  
    }
    
  _lclose(hfReadFile);

  return;  
}

VOID SaveMaxFreqAFM(HWND hWnd) {
  HFILE hfReadFile;
  UINT i;
  //struct freqMaxTag curMax; 
  
  hfReadFile = _lcreat("freqmax.dat", 0);

  if ( hfReadFile == HFILE_ERROR )
      MessageBox( hWnd, "file open error (w)", " ",MB_ICONEXCLAMATION ) ;
    
  if ( _lwrite(hfReadFile,&(scS),sizeof(scS)) != sizeof(scS) )
      MessageBox( hWnd, " file write error 2", " ",MB_ICONEXCLAMATION ) ;

  for ( i = 0; i < scS.nMax; i++ ) {
    if ( _lwrite(hfReadFile,&(afm[i]),sizeof(struct freqMaxTag)) != sizeof(struct freqMaxTag) ) {
      MessageBox( hWnd, " file write error 3", " ",MB_ICONEXCLAMATION ) ;
      _lclose(hfReadFile);
      return;
    }  
  }  
    
  _lclose(hfReadFile);

  return;  
}


UINT LoadMaxFreq(HWND hWnd) {
  HFILE hfReadFile;
  struct scanScaleTag sc;
  struct freqMaxTag curMax; 
  UINT i;
  
  if ( ( scS.pV != NULL ) || (scS.hV != NULL ) ) {
    MessageBox( hWnd, "max save error", " ",MB_ICONEXCLAMATION ) ;  
    return FALSE;
  }

  if ( ( hAFM != NULL ) || ( afm != NULL ) ) { 
    MessageBox( hWnd, " file error 2", " ",MB_ICONEXCLAMATION ) ;
    return FALSE;
  }

  hfReadFile = _lopen("freqmax.dat", READ);
               
  if ( hfReadFile == HFILE_ERROR ) {
    MessageBox( hWnd, " file error 1", " ",MB_ICONEXCLAMATION ) ;
    return FALSE;
  }  
  
  if ( _lread(hfReadFile,&(sc),sizeof(sc)) != sizeof(sc) ) {
    MessageBox( hWnd, " file error 2", " ",MB_ICONEXCLAMATION ) ;
    _lclose(hfReadFile);
    return FALSE;
  }
  
  if (initFreqData(sc.f1,sc.f2,sc.fStep) != TRUE) { 
    MessageBox( hWnd, " file error 2", " ",MB_ICONEXCLAMATION ) ;
    _lclose(hfReadFile);
    return FALSE;
  }
   
  scS.nMax = sc.nMax;
  
  hAFM = GlobalAlloc(GPTR,sc.nMax*sizeof(curMax));
  if ( hAFM == NULL ){ 
    MessageBox( hWnd, " file error 2", " ",MB_ICONEXCLAMATION ) ;
    _lclose(hfReadFile);
    return FALSE;
  }
  
  afm = (arrFreqMax)GlobalLock(hAFM);
  
  if ( afm == NULL ) {
    GlobalFree(hAFM);
    hAFM = NULL;
    MessageBox( hWnd, " file error 2", " ",MB_ICONEXCLAMATION ) ;
    _lclose(hfReadFile);
    return FALSE;
  }  

  for ( i = 0; i < sc.nMax; i++ ) {
    if ( _lread(hfReadFile,&(curMax),sizeof(curMax)) != sizeof(curMax) ) {
      MessageBox( hWnd, " file error 3", " ",MB_ICONEXCLAMATION ) ;
      GlobalUnlock(hAFM);
      GlobalFree(hAFM);
      _lclose(hfReadFile);
      return FALSE;
    }
    scS.pV[curMax.ac] = curMax.v;
    afm[i].v = curMax.v;
    afm[i].pr = curMax.pr;
    afm[i].ac = curMax.ac;
    afm[i].f = curMax.f;
  }  
    
  _lclose(hfReadFile);
  return TRUE;  
}

VOID swapAFM(short i, short j) {
  struct freqMaxTag curmax; 
  
  curmax = afm[i];
  afm[i] = afm[j];
  afm[j] = curmax;
}

VOID sortAFM( void ) {
  BOOL cont;
  UINT i;
  
  do {
    cont = FALSE;
    for (i = 1; i < scS.nMax; i++) {
      if (scS.pV[afm[i-1].ac] < scS.pV[afm[i].ac]) {
        swapAFM(i-1,i);
        cont = TRUE;
      }  
    }    
  } while (cont == TRUE);
}

VOID orderAFM( void ) {
  BOOL cont;
  UINT i;
  
  do {
    cont = FALSE;
    for (i = 1; i < scS.nMax; i++) {
      if (afm[i-1].ac > afm[i].ac) {
        swapAFM(i-1,i);
        cont = TRUE;
      }  
    }    
  } while (cont == TRUE);
}

BOOL ClearAFMPtr(HWND hWnd) {
  if ( ( hAFM == NULL ) || ( afm == NULL ) ) return FALSE;
  
  GlobalUnlock(hAFM);
  GlobalFree(hAFM);
  return TRUE;
}
