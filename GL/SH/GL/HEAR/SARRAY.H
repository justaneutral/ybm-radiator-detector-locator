#define mqueue 200 // размер массива
//#define motstup 5  // место, из которого беру уровень тишины

extern double queuelevel;

//static double f[mqueue]; // сортируемые величины
  // индексы - для циклического добавления / удаления
//static int p[mqueue], // положение в f.
//          bp[mqueue]; // обратная ссылка.
//static tp = 0; // циклически изменяемый указатель в p[].

VOID initqueue( double f0 );
VOID addqueue( double f0 );
BOOL testqueue( void );

