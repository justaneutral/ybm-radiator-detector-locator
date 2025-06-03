#include "cinc.c"

typedef struct maxRecordTag {
  long f1,f2,fm;
  char v;
} MAXRECORD;

#define mMax 200

// static HFILE hfDataFile;

static char *fileName = "mx.dat";

static MAXRECORD *bf; // буфер для буферизации работы с файлом
static short tBf;     // указатель в буфере

static BOOL rScanError = FALSE;

static VOID SaveBuffer( HWND hWnd, HFILE hfDataFile ) {
  if ( rScanError )
    return;
    
  if ( _lwrite(hfDataFile,bf,tBf*sizeof(MAXRECORD)) != tBf*sizeof(MAXRECORD) ) {
        Message( hWnd, " file write error " ) ;
        _lclose(hfDataFile);
        rScanError = TRUE;
        return;
      }  
  
  tBf = 0;
}


static VOID RewriteFile( HWND hWnd, HFILE hfDataFile ) {
  if ( rScanError )
    return;

  tBf = 0;
  hfDataFile = _lcreat(fileName, 0);

  if ( hfDataFile == HFILE_ERROR ) {
    Message( hWnd, "file create error" ) ;
    rScanError = TRUE;
    return;
  }    
}


static VOID CloseFile( HWND hWnd, HFILE hfDataFile ) {

  if ( rScanError )
    return;

  if ( tBf != 0 ) {
    SaveBuffer(hWnd);
    if ( rScanError )
      return;
  }
  
  if ( _lclose(hfDataFile) != 0 ) {
    rScanError = TRUE;
    return;
  }  
}


static VOID AddToFile( MAXRECORD *r, HFILE hfDataFile ) {
  if ( rScanError )
    return;

  if ( tBf >= MAXRECORD )
    SaveBuffer();
    
  bf[tBf] = *r;
  
  tBf++;
}

#define mV 17

long pos[mV], neg[mV];

enum chStateTag (gt,ge,lt,le) chState = gt;

long predf;
char predv;

VOID NewMesurement( long f, char v ) {

  if ( v > predv ) {
    if ( chState == gt ) {
    }
    else if ( chState == ge ) {
    }
    else if ( chState == lt ) {
    }
    else if ( chState == le ) {
    }
    chState = gt;
  }
  else if ( v == predv ) {
    if ( chState == gt ) {
    }
    else if ( chState == ge ) {
    }
    else if ( chState == lt ) {
    }
    else if ( chState == le ) {
    }  
  }
  else { /* v < predv */  
    if ( chState == gt ) {
    }
    else if ( chState == ge ) {
    }
    else if ( chState == lt ) {
    }
    else if ( chState == le ) {
    }
  }
}

