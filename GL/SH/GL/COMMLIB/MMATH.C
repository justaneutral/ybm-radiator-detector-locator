#include "cinc.h"
#include <math.h>

// Мат. процедуры общего назначения.


//  простые общие процедуры

int sign( double x )
{
  if (x < 0.0) return 1;
  if (x > 0.0) return -1;
  return 0;
}


double sqr( double x ) {
  return x*x;
}


double fmax( double f1, double f2 )
{
  if ( f1 < f2 ) return f2;
  else return f1;
}


double fmin( double f1, double f2 )
{
  if ( f1 > f2 ) return f2;
  else return f1;
}

//________ Операции с массивами: экстремумы _________
//

short absP(DOUBLEARRAY d) {
// Нахождение наибольшего по абсолютной велечине элемента в массиве.

  short i,im,m;
  double fm,fa;

  if ( error )
    return 0;
  
  m = getMemDC(d)->m;
  fm = fabs(d[0]);
  im = 0;
  
  for ( i = 1; i < m; i++ ) {
    fa = fabs(d[i]);
    if ( fm < fa ) {
      fm = fa;
      im = i;
    }
  }    
  return im;
}  


short minP(DOUBLEARRAY d) {
// Нахождение минимального элемента в массиве.

  short i,im,m;
  double fm,fa;

  if ( error )
    return 0;
  
  m = getMemDC(d)->m;
  fm = d[0];
  im = 0;
  
  for ( i = 1; i < m; i++ ) {
    fa = d[i];
    if ( fm > fa ) {
      fm = fa;
      im = i;
    }
  }
  
  return im;
}  


short maxP(DOUBLEARRAY d) {
// Нахождение максимального элемента в массиве.
  short i,im,m;
  double fm,fa;

  if ( error )
    return 0;
  
  m = getMemDC(d)->m;
  fm = d[0];
  im = 0;
  
  for ( i = 1; i < m; i++ ) {
    fa = d[i];
    if ( fm < fa ) {
      fm = fa;
      im = i;
    }
  }

  return im;
}  
  
// ________ Операции с double массивами (продолжение). ________

VOID vtrend(DOUBLEARRAY d) 
// вычитание тренда.
{
  short i,m;
  double t;
  
  if ( error )
    return;

  t = 0.0;
  m = getMemDC(d)->m;

  if ( m < 1 )
    return;
    
  for ( i = 0; i < m; i++ ) 
    t += d[i];
  
  t /= m;
  
  for ( i = 0; i < m; i++ ) 
    d[i] -= t;
}


VOID normirA(DOUBLEARRAY d) 
// Нормировка функции: ( 0.0 <= d[] <= 1.0 ).
{
  short i,m;
  double t,m1,m2;

  #define fMin 0.00000001
  
  if ( error )
    return;
  
  m = getMemDC(d)->m;
  m1 = d[0];
  m2 = d[0]+fMin;
  
  for ( i = 0; i < m; i++ ) {
    t = d[i];
    if ( m1 > t ) m1 = t;
    if ( m2 < t ) m2 = t;
  }
  
  m2 -= m1;
  
  for ( i = 0; i < m; i++ ) 
    d[i] = (d[i]-m1)/m2;
}


void initXemm( DOUBLEARRAY d )
// инициализация массива окном Хемминга
{
  int	i,l;
  double tx;
  
  if ( error )
    return;

  l = getMemDC(d)->m;
  tx = 6.283185303/l;
  
  for ( i = 0; i < l; i++ )
    d[i] = (0.54-0.46*cos(tx*i));
}


void corr( DOUBLEARRAY sgnl, DOUBLEARRAY corr )
// автокорреляция.
// corr = корреляция (sgnl);
{
  short i,j,nC,nS;
  double s;
  
  if ( error )
    return;

  nC = getMemDC(corr)->m;
  nS = getMemDC(sgnl)->m;
  
  for ( i = 0; i < nC; i++ ) {
    s = 0.0;
    
    for ( j = i; j < nS; j++ ) 
      s += sgnl[j-i]*sgnl[j];
    
    corr[i] = s;
  }
}


VOID Corrff( DOUBLEARRAY f1, DOUBLEARRAY f2, DOUBLEARRAY c )
//
// Одностороняя корреляция.
// 
// c = корреляция (f1,f2);
{
  short i,j,mC,mS,mS2;
  double s;
  
  if ( error )
    return;

  mC = getMemDC(c)->m;
  mS = getMemDC(f1)->m;
  mS2 = getMemDC(f2)->m;
  
  if ( mS != mS2 ) {
    error = TRUE;
    return;
  }
    
  for ( i = 0; i < mC; i++ ) {
    s = 0.0;
    
    for ( j = i; j < mS; j++ ) 
      s += f1[j-i]*f2[j];
      
    c[i] = s;
  }
}


VOID Corrbff( DOUBLEARRAY f1, DOUBLEARRAY f2, DOUBLEARRAY c )
//
// Двустороняя корреляция.
// 
// c = корреляция (f1,f2);
// mCH - ноль.
{
  short i,j,i1,mC,mS,mS2,mCH;
  double s;
  
  if ( error )
    return;

  mC = getMemDC(c)->m;
  mS = getMemDC(f1)->m;
  mS2 = getMemDC(f2)->m;
  
  if ( mS != mS2 ) {
    error = TRUE;
    return;
  }
  
  mCH = mC/2;
    
  for ( i = 1; i <= mCH; i++ ) {
    s = 0.0;
    
    for ( j = i; j < mS; j++ ) 
      s += f2[j-i]*f1[j];
      
    c[mCH-i] = s;
  }

  for ( i1 = mCH; i1 < mC; i1++ ) {
    i = i1-mCH;
    s = 0.0;
    
    for ( j = i; j < mS; j++ ) 
      s += f1[j-i]*f2[j];
      
    c[i1] = s;
  }
}


VOID vmul( DOUBLEARRAY aRes, DOUBLEARRAY a1, DOUBLEARRAY a2 ) {
// Умножение: aRes = a1*a2
  short i,m1,m2,m3;
  
  if ( error )
    return;

  m1 = getMemDC(a1)->m;
  m2 = getMemDC(a2)->m;
  m3 = getMemDC(aRes)->m;

  if ( (m1 != m2) or (m1 != m3) ) {
    error = TRUE;
    return;
  }
  
  for ( i = 0; i < m1; i++ )
    aRes[i] = a1[i]*a2[i];
}    
      

VOID vdiv( DOUBLEARRAY aRes, DOUBLEARRAY a1, DOUBLEARRAY a2 ) {
// Деление: aRes = a1/a2
  short i,m1,m2,m3;
  
  if ( error )
    return;

  m1 = getMemDC(a1)->m;
  m2 = getMemDC(a2)->m;
  m3 = getMemDC(aRes)->m;

  if ( (m1 != m2) or (m1 != m3) ) {
    error = TRUE;
    return;
  }
  
  for ( i = 0; i < m1; i++ )
    aRes[i] = a1[i]/a2[i];
}    


VOID vadd( DOUBLEARRAY aRes, DOUBLEARRAY a1, DOUBLEARRAY a2 ) {
// Сложение: aRes = a1+a2

  short i,m1,m2,m3;
  
  if ( error )
    return;

  m1 = getMemDC(a1)->m;
  m2 = getMemDC(a2)->m;
  m3 = getMemDC(aRes)->m;

  if ( (m1 != m2) or (m1 != m3) ) {
    error = TRUE;
    return;
  }
  
  for ( i = 0; i < m1; i++ )
    aRes[i] = a1[i]+a2[i];
}    


VOID vsub( DOUBLEARRAY aRes, DOUBLEARRAY a1, DOUBLEARRAY a2 ) {
// Вычитание: aRes = a1-a2
  short i,m1,m2,m3;
  
  if ( error )
    return;

  m1 = getMemDC(a1)->m;
  m2 = getMemDC(a2)->m;
  m3 = getMemDC(aRes)->m;

  if ( (m1 != m2) or (m1 != m3) ) {
    error = TRUE;
    return;
  }
  
  for ( i = 0; i < m1; i++ )
    aRes[i] = a1[i]-a2[i];
}    


VOID vequC( DOUBLEARRAY aRes, double c ) {
// Присваивание: aRes = c
  short i,m;
  
  if ( error )
    return;

  m = getMemDC(aRes)->m;
  
  for ( i = 0; i < m; i++ )
    aRes[i] = c;
}    


VOID vabs( DOUBLEARRAY aRes, DOUBLEARRAY a1) {
// Модуль: aRes = fabs(a1)
  short i,m1,m2;

  if ( error )
    return;
  
  m1 = getMemDC(a1)->m;
  m2 = getMemDC(aRes)->m;

  if ( m1 != m2 ) {
    error = TRUE;
    return;
  }
  
  for ( i = 0; i < m1; i++ )
    aRes[i] = fabs(a1[i]);
}    


VOID vequ( DOUBLEARRAY aRes, DOUBLEARRAY a1 ) {
// Присваивание: aRes = a1.

  short i,m1,m2;
  
  if ( error )
    return;

  m1 = getMemDC(a1)->m;
  m2 = getMemDC(aRes)->m;

  if ( m1 != m2 ) {
    error = TRUE;
    return;
  }
  
  for ( i = 0; i < m1; i++ )
    aRes[i] = a1[i];
}    


// **************************************************
//                     LPC
// **************************************************

typedef double raum [map+1];

int autoc( double FAR *aa, double FAR *rc0 )
// Вычисление LPC.
// Входной параметр rc0 - автокорелляционная функция
// Выходной параметр aa - коэффициенты LPC.
{
  double     s,alpha,at;
  raum       rc,rr;
  int        i,mh,minc,mincp,ip,ib;

  if ( error )
    return FALSE;

  for (i = 2; i <= map; i++ ) {
    aa[i] = 0.0;
  }

  for (i = 1; i <= map; i++ ) {
    rr[i] = rc0[i-1];
  }
  
  aa[1] = 1.0;

  if (rr[1] < 1.0)
    return FALSE;
  if ( rr[1] <= rr[2] )
    return FALSE;

  rc[1] = -rr[2]/rr[1];
  aa[2] = rc[1];
  alpha = rr[1]+rr[2]*rc[1];
  for ( minc = 2; minc <= ma; minc++ ) {
    s = 0.0;
    mincp = minc+2;
    for ( ip = 1; ip <= minc; ip++ )  s += rr[mincp-ip]*aa[ip];

    rc[minc] = -s/alpha;
    mh = minc / 2+1;
    for ( ip = 2; ip <= mh; ip++ ) {
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

//
//  F F T
//
//_______________________________________________
//

DOUBLEARRAY  fftX, fftY, fftXemm;

static short  permt[mFft];

static	DOUBLEARRAY	cost, sint;

static VOID initCosSin( void )
// инициализация синусов и косинусов для fft.
{
  int i;
  double arg,scl;

  if ( error )
    return;

  arg = 0.0;
  scl = pi2/mFft;

  for ( i = 0; i < mFft; i++ ) {
    cost[i] = cos(arg);
    sint[i] = sin(arg);
    arg = arg+scl;
  }
}


static VOID initPermut ( void ) {
// инициализация перестановок для fft.

  int
    i,j,k,nv2,mFftm1,t1;

  if ( error )
    return;

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


BOOL fftOn ( HWND hWnd, BOOL St ) {
// Размещение и инициализация / закритие массивов для fft.
// St - (TRUE / FALSE) - On / Off.
  if ( error )
    return FALSE;
  
  doubleOn ( hWnd, &fftX, mFft, St ); 
  doubleOn ( hWnd, &fftY, mFft, St );
  doubleOn ( hWnd, &cost, mFft, St );
  doubleOn ( hWnd, &sint, mFft, St );
  doubleOn ( hWnd, &fftXemm, mFft, St );
  
  if ( St ) {
    initCosSin( );
    initPermut( );
    initXemm( fftXemm );
  }  
}


static void perm1 ( void ) {
// перестановка элементов fftX, fftY для fft.
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


VOID fft ( void ) {
// преобразование Фурье (fftX, fftY).
  int targ,darg,lo,li,j1,j2,lix,lm,lmx;
  double s,c,t1,t2;

  if ( error )
    return;

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
// обратное Фурье (fftX, fftY).
  int targ,darg,lo,li,j1,j2,lix,lm,lmx,i;
  double s,c,t1,t2,cf;
  
  if ( error )
    return;

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


void fftAmp( void ) {
// Вычисление амплитуды FFT.
  int i,i2;
  double t1;
  
  if ( error )
    return;

  for ( i = 1; i < mFftH; i++ ) {
    i2 = mFft-i;
    t1      = sqr(fftX[i]+fftX[i2])+sqr(fftY[i]-fftY[i2]);
    fftY[i] = sqr(fftX[i]-fftX[i2])+sqr(fftY[i]+fftY[i2]);
    fftX[i] = t1;
  } 
  
  fftX[0] = sqr(fftX[0]);
  fftY[0] = sqr(fftY[0]);

  for ( i = mFftH; i < mFft; i++ ) {
    fftY[i] = 0.0;
    fftX[i] = 0.0;
  }

}


void planeArr( double arr[] ) {
// Сглаживание массива
  int i;
  fftArr fa;
  
  for ( i = 2; i < mFftH; i++ ) 
    fa[i-1] = 0.3*(arr[i-2]+arr[i])+0.4*arr[i-1];
  for ( i = 2; i < mFftH; i++ ) 
    arr[i-1] = fa[i-1];    
}


void logAmp( void ) {
// Логарифмирование fftX, fftY.
  int i;
  #define fMin 0.00000001
  
  if ( error )
    return;

  for ( i = 0; i < mFftH; i++ ) {
    if (fftX[i] < fMin)
      fftX[i] = fMin;
    fftX[i] = -log(fftX[i]);

    //fftX[mFftH+i] = fftX[0];

    if (fftY[i] < fMin) 
      fftY[i] = fMin;
    fftY[i] = -log(fftY[i]);
    
    fftX[mFftH+i] = fftY[i];
  }
}

// ________ End Of FFT ________


// ------ S T R E A K ------- 

//#define	l_gl		2
//int	stepB,lpoza;

#define mStreak 21
static  double  dstreak[mStreak];


static  double	streak( double xin ) {
  double ep, em, xk, den, emold;
  int    i, m = 20, mp1;

  ep = xin;
  em = dstreak[1];
  dstreak[1] = ep;
  mp1 = m+1;
  for ( i = 2; i <= mp1; i++ ) {
  	den = em*em+ep*ep;
	if ( den < 0.0001 ) den = 1.;
	xk = -2.*em*ep/den;
	emold = em;
	em = dstreak[i];
	dstreak[i] = xk*ep + emold;
	ep = ep+xk*emold;
  }
  return ep;
}


VOID vStreakFilter( DOUBLEARRAY d ) {
// Выделение возбуждающей функции
  short i, m;

  if ( error )
    return;

  loop ( i, mStreak ) 
	dstreak[i] = 0.0;
	
  m = getMemDC(d)->m;
   
  loop ( i, m )
    d[i] = streak(d[i]);
}

// ------ LOW Frequency FILTER ------

VOID vLowFilter( DOUBLEARRAY d ) {
// Низкочастотный фильтр
  double td,d2,d3,d4;
  double  d1,
  a2 = -2.34036589,
  a3 = 2.01190019,
  a4 = -0.614109218,
  p1 = 0.0357081667,
  p2 = -0.0069956244;
  short i,m;

  if ( error )
    return;

  d2 = d3 = d4 = 0.0;
  
  m = getMemDC(d)->m;
   
  loop ( i,m ) {
    td = d[i];
	d1 = td-(a4*d4+a3*d3+a2*d2);
	d4 = d3;
	d3 = d2;
	d2 = d1;
	td = (d4+d1)*p1+(d3+d2)*p2;
	d[i] = td;
  }	
}

