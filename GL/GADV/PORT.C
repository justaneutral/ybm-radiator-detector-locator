#include "port.h"
#include <conio.h>
//int PortVal = 0;

static void outinth(int port, int oi)
{
unsigned int j;
unsigned char tdat;
union {
	struct {
	unsigned dath : 8;
	unsigned datl : 8;
		} d;
	unsigned dat;
	} t;

	t.d.dath = 0;
	_outp(port--,t.d.dath);
	j = oi;
	tdat = j >> 2;
	_outp(--port,tdat);
	port += 2;
	tdat = j & 3;
	_outp(port,( tdat | 8 ));
}

static int r[8] = {    0,  128, 32, 256, 64,   0, 0, 0 };

#ifdef NDEBUG
double delayD(long ms) {
  long i;
  double s,r;
  
  s = 0.0;
  for (i = 0; i < ms*3; i++) {
    r = i;
    s += sin(r);
  }
  
  return s; 
}

void delay(long ms) {
  delayD(ms);
}

#else
void delay(long ms) {
  long i;
  
  for (i = 0; i < ms*82; i++);
}
#endif


static void outint(int port, int oi) {
  //PortVal = PortVal ^ r[oi];
  outinth(port,r[oi]);
  delay ( 8 );
  outinth(port,0);
}

static void outint1(int port, int oi) {
  short i;
  //PortVal = PortVal ^ r[oi];
  
  outinth(port,0);
  for ( i = 30000; i < 32000; i++ ) 
    outinth(port,i /*r[(i % 5)]*/);
    
  outinth(port,0);
}

void OutPort( int i ) {
  int port;
  
  port = 888; // peek(0, 0x408);
  outint(port+2,i);
  return;
}


