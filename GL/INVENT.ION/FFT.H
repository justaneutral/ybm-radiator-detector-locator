

/*------- F F T . H ---------*/

#define pFft	8
//#define mBlock	64
#define mFft	256	/* Size Of Data Block = (2**pFft) */ 
#define mFftH   128	/* mFft/2 */

#define pi2	6.283185303
#define pi	3.1415926

typedef	double	fftArr[mFft];
typedef double  fftArrH[mFftH];

extern	fftArr  fftX,fftY;
extern  fftArrH sXY,sXYG;

void	initFft( void );	/* init	*/
double	sqr( double x );	/* x*x	*/
void	fft( void );		/* fft	*/
void	fftm( void );		/* back	*/
void	fftAmp( void );     /* abs  */
void	planeArr( double arr[] );  /* mid, log */
void	logAmp( void );

/*------ X E M M . H ----------*/

//#define fMin 			0.0001
#define NSAMPLE_CH	   	mFft

typedef double	blockArr[NSAMPLE_CH];
extern  blockArr xemm;

//void    initXemm( void );

/* LPC.H */

int getpar( double sgnl[] );
short round ( double x );

double CopyDataCh( int nom );

void   initXemm( void );
