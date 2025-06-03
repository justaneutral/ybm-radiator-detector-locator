#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include "sound.h"
#include "fft.h"

#define     ma        17
#define     map       18  // (ma+1)
#define     loop(i,j,k)   for (i=j; i<=k; i++)

typedef		double	  raum[map+1];

raum	aa;

void lpcCorr( raum corr, double sgnl[] )
{
  short i,j;
  double s;
  
  for ( i = 0; i <= map; i++ ) {
    s = 0.0;
    
    for ( j = i; j < NSAMPLE_CH; j++ ) 
      s += sgnl[j-i]*sgnl[j];
    
    corr[i+1] = s;
  }
}

int autoc( double sgnl[] ) {
  double     s,alpha,at;
  raum       rc,rr;
  int        i,mh,minc,mincp,ip,ib;

  loop (i,2,map) {
    aa[i] = 0.0;
  }
  
  lpcCorr(rr, sgnl);
  
  aa[1] = 1.0;

  if (rr[1] < NSAMPLE_CH)
    return FALSE;
  if ( rr[1] <= rr[2]+0.001 )
    return FALSE;

  rc[1] = -rr[2]/rr[1];
  aa[2] = rc[1];
  alpha = rr[1]+rr[2]*rc[1];
  loop (minc,2,ma) {
    s = 0.0;
    mincp = minc+2;
    loop (ip,1,minc)  s += rr[mincp-ip]*aa[ip];

    rc[minc] = -s/alpha;
    mh = minc / 2+1;
    loop (ip,2,mh) {
      ib = mincp-ip;
      at = aa[ip]+rc[minc]*aa[ib];
      aa[ib] = aa[ib]+rc[minc]*aa[ip];
      aa[ip] = at;
    }
    aa[minc+1] = rc[minc];

    alpha += rc[minc]*s;
    if (alpha <= 0.000001) return FALSE;
  }
  return TRUE;
}

/*
double dft(int i, int lm, raum a) {
  int j,step,tpos;
  double cs,sn;

  step = i+1;
  tpos = step;
  sn = 0.0;
  cs = 1.0;
  loop (j,2,lm) {
    sn = sn+a[j]*rsin[tpos];
    cs = cs+a[j]*rcos[tpos];
    tpos = tpos+step;
    if (tpos >= namp2) tpos -= namp2;
  }
  cs = cs*cs+sn*sn;
  if (cs < 0.00001) cs = 0.00001;
  return( -log(cs));
}


void inidft( void ) {
  int   i;
  double tx;

  namp = 32;
  namp2 = namp*2;
  tx = 6.2831853/namp2;
  loop (i,0,namp2) {
    rcos[i] = cos(tx*i);
    rsin[i] = sin(tx*i);
  }
}
*/

int getpar( double sgnl[] ) {
  int		i;
    
  if (autoc(sgnl) != TRUE)
    return FALSE;

  for ( i = 0; i < mFft; i++ ) {
    fftX[i] = 0.0;  
    fftY[i] = 0.0;
  }
  
  for ( i = 1; i <= map; i++ ) 
    fftX[i-1] = aa[i];

  fft();

  fftAmp();  

  return TRUE;
}

//== copy data + ...

blockArr xemm;

void initXemm( void ) {
  int	i;
  double tx;

  tx = 6.283185303/NSAMPLE_CH;
  
  for ( i = 0; i < NSAMPLE_CH; i++ )
    xemm[i] = (0.54-0.46*cos(tx*i));
}

int sign( double x ) {
  if (x < 0.0) return 1;
  if (x > 0.0) return -1;
  return 0;
}

double CopyDataCh( int nom ) 
{
  UINT i,i0,bl;
  LPACUBUFF_IN ab;
  double s,sn;

  ab = (LPACUBUFF_IN)wBuff[nom].lpWaveHdr->lpData;
  
  s = 0.0;
  sn = 0.0;
  
  for (bl = 1; bl <= 3; bl++) {
  
  i0 = NSAMPLE_IN-NSAMPLE_CH*bl;
  
  for ( i = 0; i < NSAMPLE_CH; i++ ) {
    sl[i] = (ab->acuBuff[i+i0].leftSample);
    sr[i] = (ab->acuBuff[i+i0].rightSample);
  }

  trend(sr,NSAMPLE_CH);
  trend(sl,NSAMPLE_CH);

  for ( i = 0; i < NSAMPLE_CH; i++ ) {
    sl[i] *= xemm[i];
    sr[i] *= xemm[i]; 
  }

  for ( i = 0; i < mFft; i++ ) {
    fftX[i] = sl[i];  
    fftY[i] = sr[i];
  }

  fft();
  
  fftAmp();
  
  logAmp();
  
  for ( i = 1; i < mFftH/2; i++ ) 
    s += (fftY[i]-fftY[i-1])*(fftX[i]-fftX[i-1]);

  for ( i = 1; i < mFftH/2; i++ ) {
    sn += (fftY[i]-fftY[i-1])*(fftY[i]-fftY[i-1])+
          (fftX[i]-fftX[i-1])*(fftX[i]-fftX[i-1]);
  }        
  }
  if (sn < 1.0) sn = 1.0;
  
  return s/sqrt(sn);
  //return s/sn;
}

