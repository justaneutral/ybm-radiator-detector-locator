#include <stdio.h>
#include <math.h>
#include "fft.h"

fftArr  fftX,fftY,cost,sint;

// #define fMin (-fMax)

int     permt[mFft];

void initpermut ( void ) {
  int
    i,j,k,nv2,mFftm1,t1;

  for ( i = 0; i < mFft; i++ )
    permt[i] = i;

  j = 0;
  nv2 = mFft / 2;
  mFftm1 = mFft-1;

  for ( i = 0; i < mFftm1; i++ ) {
    if ( i < j )  {
      t1 = permt[j];
      permt[j] = permt[i];
      permt[i] = t1;
    }
    k = nv2;
    while ( k <= j ) {
      j = j-k;
      k = k/2;
    }
    j = j+k;
  }
}

void perm1 ( void ) {
  int i,j;
  fftArr x1,y1;

  for ( i = 0; i < mFft; i++ ) {
    j = permt[i];
    x1[j] = fftX[i];
    y1[j] = fftY[i];
  }

  for ( i = 0; i < mFft; i++ ) {
    fftX[i] = x1[i];
    fftY[i] = y1[i];
  }
}

void initFft ( void ) {
  int i;
  double arg,scl;

  arg = 0.0;
  scl = pi2/mFft;

  for ( i = 0; i < mFft; i++ ) {
    cost[i] = cos(arg);
    sint[i] = sin(arg);
    arg = arg+scl;
  }
  initpermut();
  initXemm();
}

/*
void old_permut ( void ) {
  int i,j,k,nv2,mFftm1;
  double t1,t2;

  j = 0;
  nv2 = mFft / 2;
  mFftm1 = mFft-1;
  for ( i = 0; i < mFftm1; i++ ) {
    if ( i < j ) {
      t1 = fftX[j];
      t2 = fftY[j];
      fftX[j] = fftX[i];
      fftY[j] = fftY[i];
      fftX[i] = t1;
      fftY[i] = t2;
    }
    k = nv2;
    while ( k < j ) {
      j = j-k;
      k = k / 2;
    }
    j = j+k;
  }
}
*/

void fft ( void ) {
  int targ,darg,lo,li,j1,j2,lix,lm,lmx;
  double s,c,t1,t2;

  lmx = mFft;
  darg = 1;
  for ( lo = 0; lo < pFft; lo++ ) {
    lix = lmx;
    lmx = lmx / 2;
    targ = 0;
    for ( lm = 1; lm <= lmx; lm++ ) {
      c =  cost[targ];
      s = -sint[targ];
      targ = targ+darg;
      li = lix-1;
      while ( li < mFft ) {
	j1 = li-lix+lm;
	j2 = j1+lmx;
	t1 = fftX[j1]-fftX[j2];
	t2 = fftY[j1]-fftY[j2];
	fftX[j1] = fftX[j1]+fftX[j2];
	fftY[j1] = fftY[j1]+fftY[j2];
	fftX[j2] = c*t1+s*t2;
	fftY[j2] = c*t2-s*t1;
	li += lix;
      }
    }
    darg = darg+darg;
  }
  perm1();
}

void fftm ( void ) {
  int targ,darg,lo,li,j1,j2,lix,lm,lmx,i;
  double s,c,t1,t2,cf;

  lmx = mFft;
  darg = 1;
  for ( lo = 0; lo < pFft; lo++ ) {
    lix = lmx;
    lmx = lmx / 2;
    targ = 0;
    for ( lm = 1; lm <= lmx; lm++ ) {
      c = cost[targ];
      s = sint[targ];
      targ = targ+darg;
      li = lix-1;
      while ( li < mFft ) {
	j1 = li-lix+lm;
	j2 = j1+lmx;
	t1 = fftX[j1]-fftX[j2];
	t2 = fftY[j1]-fftY[j2];
	fftX[j1] = fftX[j1]+fftX[j2];
	fftY[j1] = fftY[j1]+fftY[j2];
	fftX[j2] = c*t1+s*t2;
	fftY[j2] = c*t2-s*t1;
	li += lix;
      }
    }
    darg = darg+darg;
  }

  cf = 1.0/mFft;

  for ( i = 0; i < mFft; i++ ) {
    fftX[i] = fftX[i]*cf;
    fftY[i] = fftY[i]*cf;
  }
  perm1();
}

double sqr( double x ) {
  return x*x;
}

/*
void printArr( void ) {
  int i,mFft2;

  printf("----\n");

  mFft2 = (mFft / 2);
  i = 0;
  printf(" %10.5f %10.5f \n", fftX[i],fftY[i]);

  for ( i = 55; i < mFft2; i++ )
    printf(" %10.5f %10.5f \n", fftX[i],fftY[i]);
}
*/

void fftAmp( void ) {
  int i,i2;
  double t1;

  for ( i = 1; i < mFftH; i++ ) {
    i2 = mFft-i;
    t1      = sqr(fftX[i]+fftX[i2])+sqr(fftY[i]-fftY[i2]);
    fftY[i] = sqr(fftX[i]-fftX[i2])+sqr(fftY[i]+fftY[i2]);

    // if (t1 < fMin) t1 = fMin; // for sXY
    fftX[i] = t1;
  }
  fftX[0] = sqr(fftX[0]);
  fftY[0] = sqr(fftY[0]);
}

void planeArr( double arr[] ) {
  int i;
  fftArr fa;
  
  for ( i = 2; i < mFftH; i++ ) 
    fa[i-1] = 0.3*(arr[i-2]+arr[i])+0.4*arr[i-1];
  for ( i = 2; i < mFftH; i++ ) 
    arr[i-1] = fa[i-1];    
}

void logAmp( void ) {
  int i;
  #define fMin 0.00000001
  
  for ( i = 0; i < mFftH; i++ ) {
    if (fftX[i] < fMin)
      fftX[i] = fMin;
    fftX[i] = -log(fftX[i]);

    if (fftY[i] < fMin) 
      fftY[i] = fMin;
    fftY[i] = -log(fftY[i]);
  }
}

/*
void t( void ) {
  int i;

  inittab();

  for ( i = 0; i < mFft; i++ ) {
    fftX[i] = 7.0+1.5*cos(pi2/mFft*61.0*i+pi/2)+3.5*cos(pi2/mFft*62.0*i);
    fftY[i] = 1.1+4.5*cos(pi2/mFft*63.0*i+pi/5);
  }

  printArr();
  fft();
  d();

  // fftm();
  // printArr();
  // fft_p(fftX,fftY,mFft);
  // d();
}

int main ( void ) {
  t();
  return 0;
}
*/
