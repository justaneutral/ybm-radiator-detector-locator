// упорядоченный список 
#include "cinc.h"

#define mqueue 50 // размер очереди 

typedef int elemId;
typedef char queuePtr;

typedef struct {
  queuePtr prev,next;
  double f;
  elemId id;
} queueElem;


queuePtr  first,last,freeL;
queueElem  queuearray[mqueue], *curqueue;


queuePtr bestqueue( void );
VOID insertqueue ( double newF, elemId newId );
VOID initqueue( void );
VOID delFirst( void );


VOID testqueue ( void ) {
  int i,j,ip;
  double fp;

  i = first;
  j = 0;
  ip = 0;

  if ( i != 0 )
    fp = queuearray[i].f;

  while ( i != 0) {
    j++;
    if ( queuearray[i].f > fp ) {
      errorstring("testqueue"); 
      return;
    }  
    fp = queuearray[i].f;
    ip = i;
    i = queuearray[i].next;
  }

  i = freeL;
  while ( i != 0) {
    j++;
    i = queuearray[i].next;
  }

  if ( j != mqueue ) {
    errorstring(" n Elem ");
    return;
  }

  if ( ip != last ) {
    errorstring(" Last ");
    return;
  }
}


VOID delFirst ( void ) {
  int j;

  if ( first == 0 ) {
    errorstring("del First");
    return;
  }  
  
  j = first;

  first = queuearray[j].next;
  
  if ( first == 0 ) {
    errorstring("del First.1");
    return;
  }  
  
  queuearray[first].prev = 0;

  if ( last == first )
    last = 0;
  
  queuearray[j].next = freeL;
  freeL = j;
}

 
VOID insertqueue( double newF, elemId newId ) {
// вставка в список 
  int i,j,k,p;
  BOOL cont;

  if ( first == 0 ) {
    if ( freeL == 0 ) {
      errorstring("freeL");
      return;
    }  

    j = freeL;
    first = j;
    last = j;
    freeL = queuearray[freeL].next;

    curqueue = queuearray+j;
    curqueue->prev = 0;
    curqueue->next = 0;
    curqueue->id = newId;
    curqueue->f = newF;
    
    return;
  }

  if ( newF <= queuearray[last].f ) {
    if ( freeL == 0 )
      return;
    p = freeL;
    freeL = queuearray[p].next;
    queuearray[last].next = p;

    curqueue = queuearray+p;
    curqueue->prev = last;
    curqueue->next = 0;
    curqueue->id = newId;
    curqueue->f = newF;

    last = p;
    return;
  }
  
  i = first;
  cont = TRUE;
  while ( cont and (i != 0) ) {
    if ( queuearray[i].f < newF )
      cont = FALSE;
    else
      i = queuearray[i].next;
  }

  if ( (i == 0) ) {
    errorstring(" < and > ");
    return;
  }

  if ( freeL != 0 ) {
    j = freeL;
    freeL = queuearray[j].next;
  }
  else {
    // вставка с вытеснением последнего 
    if ( i == last ) {
      curqueue = queuearray+i;
      curqueue->id = newId;
      curqueue->f = newF;      
      return;
    }
    else {
      k = queuearray[last].prev;
      queuearray[k].next = 0;
      j = last;
      last = k;
    } 
  }
  // вставка j перед i 
  
  curqueue = queuearray+j;
  curqueue->next = i;
  curqueue->prev = queuearray[i].prev;
  
  queuearray[i].prev = j;
  curqueue->id = newId;
  curqueue->f = newF;
  
  if ( curqueue->prev == 0 )
    first = j;
  else
    queuearray[curqueue->prev].next = j;
}


VOID insertqueueX(double newF, elemId newId) {
  insertqueue(newF,newId);
}


queuePtr bestqueue( void ) {
  if ( first == 0 )
    return -1;
  else 
    return queuearray[first].id;
}


VOID initqueue ( void ) {
  int i;

  first = 0;
  last  = 0;
  freeL = 0;

  loop (i,mqueue) {
    queuearray[i].next = freeL;
    freeL = i;
  }
}

