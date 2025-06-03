#include "cinc.h"
#include "sarray.h"
// сортируемый массив - для измерения текущего уровня шума

//#define mqueue 200 // размер массива
#define motstup 5  // место, из которого беру уровень тишины

double queuelevel;

static double f[mqueue]; // сортируемые величины
  // индексы - для циклического добавления / удаления
static int p[mqueue], // положение в f.
          bp[mqueue]; // обратная ссылка.
static tp = 0; // циклически изменяемый указатель в p[].

VOID initqueue( double f0 );
VOID addqueue( double f0 );
BOOL testqueue( void );


VOID initqueue( double f0 ) {
// инициализация
  int i;
  
  loop(i, mqueue) {
    f[i] = f0;
    p[i] = i;
    bp[i] = i;
  }
  queuelevel = f0;  
  tp = 0;
}


static void qswap( int i1, int i2 ) {
// i1,i2 - положение в массивах f, bp.
  int i;
  double d;
  
  d = f[i1];
  f[i1] = f[i2];
  f[i2] = d;
  
  i = bp[i1];
  bp[i1] = bp[i2];
  bp[i2] = i;
  
  p[bp[i1]] = i1;
  p[bp[i2]] = i2;
}


BOOL testqueue( void ) {
  int i,j,k,l;
  
  return TRUE;
  
  loop (i, mqueue-1) {
    if ( f[i+1] < f[i] )
      return FALSE;
  }
  
  loop(i, mqueue) {
    k = 0;
    l = 0;
    loop(j, mqueue) {
      if ( p[j] == i )
        k++;
      if ( bp[j] == i )
        l++;
    }
    
    if ( (k != 1) or (l != 1) )
      return FALSE;
  }
  
  loop(i,mqueue) {
    if (p[bp[i]] != i) 
      return FALSE;
    if (bp[p[i]] != i)
      return FALSE;
  }
        
  return TRUE;
}
      

VOID addqueue( double f0 ) {
// добавление очередного значения
  int i;

  tp++;
  if ( tp >= mqueue )
    tp = mqueue;
  
  i = p[tp];
  f[i] = f0;  
  
  while ( ( i > 0 ) and ( f[i-1] > f[i] ) ) {
    qswap(i, i-1);
    i--;
  }  

  while ( ( i < (mqueue-1) ) and ( f[i] > f[i+1] ) ) {
    qswap(i, i+1);
    i++;
  }  
  queuelevel = f[motstup];
}
