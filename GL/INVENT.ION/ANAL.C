#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <math.h>
#include "sound.h"
#include "resource.h"

FBUFF 	sl,sr;

HANDLE  hLeftMid,hRightMid, hLeftSample,hRightSample;
LPFBUFF pLeftMid,pRightMid, pLeftSample,pRightSample;

double corrL[MCORR]; //,corrR[MCORR];

short dBugL = 0, dBugR = 0;

/* --- Buffers --- */


UINT    lockBuffers( HWND hWnd ) {
  pLeftMid = (LPFBUFF) GlobalLock(hLeftMid);
  if (!pLeftMid) {
    messageWin(hWnd, "Lock Error.");
    return FALSE;
  }
  pRightMid = (LPFBUFF) GlobalLock(hRightMid);
  if (!pRightMid) {
    messageWin(hWnd, "Lock Error.");
    return FALSE;
  }
  pLeftSample = (LPFBUFF) GlobalLock(hLeftSample);
  if (!pLeftSample) {
    messageWin(hWnd, "Lock Error.");
    return FALSE;
  }
  pRightSample = (LPFBUFF) GlobalLock(hRightSample);
  if (!pRightSample) {
    messageWin(hWnd, "Lock Error.");
    return FALSE;
  }
  return TRUE;
}


VOID    unlockBuffers( void ) {
  GlobalUnlock(hLeftMid);
  GlobalUnlock(hRightMid);
  GlobalUnlock(hLeftSample);
  GlobalUnlock(hRightSample);
}


UINT clearBuffers( HWND hWnd ) {
  int i;
  
  if (lockBuffers(hWnd) == FALSE)
    return FALSE;
    
  for ( i = 0; i < NSAMPLE_IN; i++ ) {
    (*pLeftMid)[i] = 0;
    (*pRightMid)[i] = 0;
    //(*pLeftSample)[i] = 0;
    //(*pRightSample)[i] = 0;
  }    
  unlockBuffers();
  
  return TRUE;
}

// ----------------- Init -------------------

VOID CalcCorr( LPFBUFF f1, LPFBUFF f2, double c[] );

VOID CalcCorr( LPFBUFF f2, LPFBUFF f1, double c[] )
{
  short i,j;
  double s;
  
  
  for ( i = 0; i < MCORR; i++ ) {
    s = 0.0;
    
    for ( j = i; j < NSAMPLE_IN; j++ ) 
      s += (*f1)[j-i]*(*f2)[j];
      
    c[i] += s;
  }
}


UINT	InitAnal( HWND hWnd ) {  
  hLeftMid = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                           (DWORD) sizeof(FBUFF));
  if (!hLeftMid) {
    messageWin(hWnd, "Not enough memory.");
    return FALSE;
  }
  
  hRightMid = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                           (DWORD) sizeof(FBUFF));
  if (!hRightMid) {
    messageWin(hWnd, "Not enough memory.");
    return FALSE;
  }
  
  hLeftSample = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                           (DWORD) sizeof(FBUFF));
  if (!hLeftSample) {
    messageWin(hWnd, "Not enough memory.");
    return FALSE;
  }
  
  hRightSample = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                           (DWORD) sizeof(FBUFF));
  if (!hRightSample) {
    messageWin(hWnd, "Not enough memory.");
    return FALSE;
  }
  
  return TRUE;
  
}


VOID InitData( HWND hWnd ) {
  short i;
          
  // initXemm();
  InitAnal(hWnd);
  
  for ( i = 0; i < NSAMPLE_IN; i++ ) {
    sr[i] = 0;
    sl[i] = 0;
  }

  for ( i = 0; i < MCORR; i++ ) {
    corrL[i] = 0;
    //corrR[i] = 0;
  }
  
  clearBuffers(hWnd);

  {
  short i;
    for ( i = 0; i < NSAMPLE_IN; i++ ) {
     sr[i] = 0.0; //(*pRightSample)[i];
     sl[i] = 0.0; //(*pLeftSample)[i];
    }
  }

  //loadBuffers(hWnd);
}

//---------------------------------------------------

short round(double f) {
  return (short)(f+0.5);
}


VOID trend( double f[], short m ) {
  short i;
  double t;
  
  t = 0.0;

  for ( i = 0; i < m; i++ ) 
    t += f[i];
  
  t /= m;
  
  for ( i = 0; i < m; i++ ) 
    f[i] -= t;
    
}

VOID normirFun( double f[], short m ) {
  short i;
  double t,m1,m2;
  
  m1 = f[0];
  m2 = f[0]+fMin;
  
  for ( i = 0; i < m; i++ ) {
    t = f[i];
    if ( m1 > t ) m1 = t;
    if ( m2 < t ) m2 = t;
  }

  for ( i = 0; i < m; i++ ) 
    f[i] = (f[i]-m1)/(m2-m1);
    
}


VOID CopyData( int nom ) 
{
  UINT i;
  LPACUBUFF_IN ab;

  ab = (LPACUBUFF_IN)wBuff[nom].lpWaveHdr->lpData;
  
    for ( i = 0; i < NSAMPLE_IN; i++ ) 
      sl[i] = ab->acuBuff[i].leftSample;
    trend(sl,NSAMPLE_IN);
    
    for ( i = 0; i < NSAMPLE_IN; i++ ) 
      sr[i] = ab->acuBuff[i].rightSample;  
    trend(sr,NSAMPLE_IN);
}



VOID DoLowPass( HWND hWnd )  // Drawing's 
{
  short i;
  
      lockBuffers(hWnd);
      for ( i = 0; i < NSAMPLE_IN; i++ ) 
        (*pLeftMid)[i] = sl[i];
      for ( i = 0; i < NSAMPLE_IN; i++ ) 
        (*pLeftSample)[i] = sr[i];

      CalcCorr(pLeftMid,pLeftSample,corrL);
      
      //normirFun(corrL,MCORR);

      unlockBuffers();
}

short absD(double co[], short mCo ) {
  short i,im;
  double fm,fa;
  
  fm = fabs(co[0]);
  im = 0;
  
  for ( i = 1; i < mCo; i++ ) {
    fa = fabs(co[i]);
    if ( fm < fa ) {
      fm = fa;
      im = i;
    }
  }    
  return im;
}  

short minD(double co[], short mCo ) {
  short i,im;
  double fm;
  
  fm = co[0];
  im = 0;
  
  for ( i = 1; i < mCo; i++ )
    if ( fm > co[i] ) {
      fm = co[i];
      im = i;
    }
      
  return im;
}  

short maxD(double co[], short mCo ) {
  short i,im;
  double fm;
  
  fm = co[0];
  im = 0;
  
  for ( i = 1; i < mCo; i++ )
    if ( fm < co[i] ) {
      fm = co[i];
      im = i;
    }
      
  return im;
}  
  
double minV(double co[], short mCo ) {
  short i,im;
  double fm;
  
  fm = co[0];
  im = 0;
  
  for ( i = 1; i < mCo; i++ )
    if ( fm > co[i] ) {
      fm = co[i];
      im = i;
    }
      
  return fm;
}  

double maxV(double co[], short mCo ) {
  short i,im;
  double fm;
  
  fm = co[0];
  im = 0;
  
  for ( i = 1; i < mCo; i++ )
    if ( fm < co[i] ) {
      fm = co[i];
      im = i;
    }
      
  return fm;
}  

short maxDD(double f[], short mCo ) {
  short i,im,mode;
  double fm,fp,fpn;
  
  fm = f[0];
  fp = 0.0;
  fpn = fp;
  
  im = 0;
  
  mode = -1;
  
  if (f[0] > 0.0)
    mode = 1;
  
  for ( i = 0; i < mCo; i++ ) {
    if ( mode > 0 ) {
      if ( f[i] < 0.0 )  {
        fp = fpn;
        fpn = -f[i];
        mode = -1;
      }
      else {
        if ( fpn < f[i] )
          fpn = f[i];
        if ( fm < (f[i]+fp) ) {
          im = i;
          fm = f[i]+fp;
        }    
      }  
    }
    else {
      if ( f[i] > 0.0 ) {
        fp = fpn;
        fpn = -f[i];
        mode = 1;
      }
      else {
        if ( fpn < -f[i] )
          fpn = -f[i];
        if ( fm < (-f[i]+fp) ) {
          im = i;
          fm = -f[i]+fp;
        }    
      }  
    }
  }
  return im;
}  

