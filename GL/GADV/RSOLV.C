#include "cinc.h"
#include "math.h"
#include "stdlib.h"

double r[4];   // рассто€ни€ до вершин

double v[4];   // solution & nevias

#define DJ 45.0

#define sq125  1.11803398875 
#define sq149  0.6666666666667

double r2[4];   // рассто€ни€ до вершин
   
static double        // координаты вершин
   mtr[4][3] = {  { -0.5*DJ,        0.0,       0.0},  
                  {  0.5*DJ,        0.0,       0.0},
                  {  0.0,      sq125*DJ,       0.0},
                  {  0.0,  sq125/3.0*DJ,  sq149*DJ}};

static double d[4];  // нев€зка

static double nevias( double tv[] ) {
  int i,j;
  double s,sl;

  s = 0.0;
  loop(i,4) {      // цикл по строкам  
    sl = 0.0;
    loop(j,3) {    //      по столбцам 
      sl += sqr(tv[j]-mtr[i][j]);  // вычисление нев€зки
    }  
    s += sqr(sl-r2[i]);
  }

  return s;
}

static BOOL randomStep( double range ) {
  int i,j;
  double vn[3],sn,ds, s;
  BOOL newVal;
  
  s = nevias(v);
  
  ds = range/RAND_MAX;
  
  newVal = FALSE;
  
  loop(i,200) {     
    loop(j,3) {
      sn = rand();
      vn[j] = v[j]-range*0.5+sn*ds;
    }
    sn = nevias(vn);
    if (sn < s) {
      loop(j,3)
        v[j] = vn[j];
      s = sn;
      newVal = TRUE;
      v[3] = sqrt(s);
    }
  }

  return newVal;  
}

VOID rsolv( void ) {
  int i;
  double s;

  s = 200.0;
  
  loop(i,3)
    v[i] = 0.0;

  loop(i,4)
    r2[i] = r[i]*r[i];
    
  v[3] = 1000.0;  
    
  loop(i,10) {
    while ( randomStep(s) ) ;
    s *= 0.5;
  }  
}
