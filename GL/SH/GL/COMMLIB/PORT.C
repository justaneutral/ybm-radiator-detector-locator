#include <windows.h>
#include "port.h"
#include <conio.h>
#include <math.h>
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


//static int r[8] = {    0,  128, 32, 256, 64,   32+64+128+256, 0, 0 };
static int r[] = {    0,  128, 64, 256, 32,
                      //32+64+128+256,    //5
                      //1+2+4+16,         //6
                      128+256,          //7
                      32+64 };          //8

double delay_ret(long ms) {
  long i;
  double s;
  
  s = 0.0;
#ifdef NDEBUG
  for (i = 0; i < ms*24; i++)
    s += sin(i);
#else
  for (i = 0; i < ms*14; i++)
    s += sin(i);
#endif
  return s;
}


void delay(long ms) {
  delay_ret(ms);
}  


static void outint(int port, int oi) {
  //PortVal = PortVal ^ r[oi];
  outinth(port,r[oi]);
  delay ( 3 );
  outinth(port,0);
}

VOID setport(int i) {
  outinth(888+2,r[i]);
}  

static void outint1(int port, int oi) {
  short i;
  //PortVal = PortVal ^ r[oi];
  
  outinth(port,0);
  for ( i = 30000; i < 32000; i++ ) 
    outinth(port,i /*r[(i % 5)]*/);
    
  outinth(port,0);
}

void outport( int i ) {
  int port;
  
  port = 888; // peek(0, 0x408);
  outint(port+2,i);
}


