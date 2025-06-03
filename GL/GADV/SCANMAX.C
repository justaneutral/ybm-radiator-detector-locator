#include "cinc.h"
#include "fcomm.h"
#include "string.h"
#include "scan.h"

// Сканирование

// Выделение максимумов и запись их в массив.


enum  { MaxGt, MaxGe, MaxLt } MaxState = MaxLt;

enum  ScanCommand { ScanSetFreq, ScanGetVFreq, ScanEnd, ScanInit } ScanMode;

//BOOL  ScanMode  = FALSE;

static BOOL
      EmptyPV = TRUE, // признак, что pVFreq не заполнен.
      EmptyTV = TRUE; // признак, что текущее значение сигнала 
                      //    измеряется в первый раз.

FreqVal tV;
static FreqVal pV;      // Величина сигнала в предыдущей точке.
static FreqVal PrevBig; // Для сохранения значения при переходе 
                        //    с большого шага на маленький.
static vFreq ptVv = 0;  // Предыдущее значение сигнала в текущей точке
                        // (для ожидания, пока она стабилизируется).

static freq tSmallFreq = 100;

freq
      BigStep    = 300,
      SmallStep  = 150;

FreqMaxRec tMax;

extern FreqTab CurTab = { 0, NULL, 100, 0 }; 

VOID ClearMaxScan( LPFreqTab ft ) {

  if ( CommonError )
    return;
    
  MaxState = MaxLt;
  ScanMode = ScanInit;
  
  ft->nMax = 0;
  
  EmptyTV = TRUE;
  EmptyPV = TRUE;
  
  tV.f = CurTab.FirstFreq;
  tV.v = 0;
  pV = tV;
  ptVv = 0;
  PrevBig.f = 0;
  PrevBig.v = 0;
  
  tSmallFreq = CurTab.FirstFreq;
  tMax.fMid.v = 0;
  
}


BOOL FreqScaleOn( LPFreqTab ft, BOOL St ) {
  
  if (!MemoryOn(&(ft->FreqMax), mFreqMax*sizeof(FreqMaxRec), St)) {
    CommonError = TRUE;
    return FALSE;
  }
  
  return TRUE;  
}


VOID SaveFreqScale(HWND hWnd, LPCSTR s, LPFreqTab ft) {
  HFILE hfReadFile;
  long tm;
  
  if ( CommonError )
    return;
    
  hfReadFile = _lcreat(s, 0);

  if ( hfReadFile == HFILE_ERROR ) {
    Message( hWnd, "Can't create file." ) ;
    CommonError = TRUE;
    return;
  }  
    
  if ( _lwrite(hfReadFile,ft,sizeof(*ft)) != sizeof(*ft) ) {
    Message(hWnd,"Can't write file.");
    CommonError = TRUE;
    return;    
  }
  
  loop (tm, ft->nMax) {
    if ( _lwrite(hfReadFile, &(ft->FreqMax[tm]),
         sizeof(FreqMaxRec)) != sizeof(FreqMaxRec) ) {
      Message(hWnd,"Can't write file.");
      CommonError = TRUE;
      return;    
    }
  }  
    
  _lclose(hfReadFile);

  return;  
}


VOID LoadFreqScale(HWND hWnd, LPCSTR s, LPFreqTab ft) {
  HFILE hfReadFile;
  FreqTab cft;
  long tm;
  
  if ( CommonError )
    return;
    
  if ( ft->FreqMax == NULL ) {
    CommonError = TRUE;
    return;
  }
    
  hfReadFile = _lopen(s, READ);

  if ( hfReadFile == HFILE_ERROR ) {
    Message( hWnd, "Can't open file." ) ;
    CommonError = TRUE;
    return;
  }
  
  if ( _lread(hfReadFile,&cft,sizeof(cft)) != sizeof(cft) ) {
    Message(hWnd,"Can't read file.");
    CommonError = TRUE;
    return;    
  }
  
  ft->nMax = cft.nMax;
  ft->FirstFreq = cft.FirstFreq;
  ft->LastFreq = cft.LastFreq;
  
  if ( ft->nMax > mFreqMax ) {
    Message(hWnd,"File Structure Error.");
    ft->nMax = 0;
    CommonError = TRUE;
    return;    
  }
  
  loop( tm, ft->nMax ) {
    if ( _lread(hfReadFile,&(ft->FreqMax[tm]),
         sizeof(FreqMaxRec)) != sizeof(FreqMaxRec) ) {
      Message(hWnd,"Can't read file.");
      CommonError = TRUE;
      return;    
    }
  }  
    
  _lclose(hfReadFile);

  return;  
}


VOID swapFreqMax(long i, long j) {
  
}

VOID sortFreqMax( LPFreqTab ft ) {
  BOOL cont;
  long i;
  FreqMaxRec curmax; 
  
  do {
    cont = FALSE;
    for (i = 1; i < ft->nMax; i++) {
      if (ft->FreqMax[i-1].power < ft->FreqMax[i].power) {
        // Swap (i-1,i);
        
        curmax = ft->FreqMax[i];
        ft->FreqMax[i] = ft->FreqMax[i-1];
        ft->FreqMax[i-1] = curmax;
        
        cont = TRUE;
      }  
    }    
  } while (cont == TRUE);
}


VOID repMax( HWND hWnd ) {
  HDC rDC;

  if ( CurTab.nMax < 1 )
    return;
    
  if ( not fwScale.On )
    return;
    
  rDC = GetDC(fwScale.hWnd);
  drawFreqMax(rDC, CurTab.nMax-1, CurTab.nMax);
  ReleaseDC( fwScale.hWnd, rDC );
}


VOID NextFVp( HWND hWnd ) {
  // обработка следующего значения величины сигнала
  if ( EmptyPV ) {
    EmptyPV = FALSE;
    return;
  }
  
  if ( ( MaxState == MaxLt ) and (( tV.v > pV.v ) or ( tV.v == 0)) ) {
    if (tMax.fMid.v > 0) {
      // новый максимум
      tMax.fMax = pV;
      if ( CurTab.nMax < mFreqMax ) {
        CurTab.nMax++;
        tMax.power = tMax.fMid.v;
        CurTab.FreqMax[CurTab.nMax-1] = tMax;
      }
      repMax( hWnd );
      freqReport(hWnd);   
    }    
      
    MaxState = MaxGt;
    tMax.fMin = tV;
    return;
  }
  
  if ( ( MaxState == MaxGt ) and ( tV.v == 0 ) ) {
    //MaxState = MaxLt;
    tMax.fMin = tV;
    return;
  }

  if ( ( MaxState == MaxGt ) and ( tV.v < pV.v ) ) {
    MaxState = MaxLt;
    tMax.fMid = pV;
    return;
  }
  
}


VOID NextFV( HWND hWnd ) {
  NextFVp(hWnd);
  pV = tV;
}


BOOL ScanStep( HWND hWnd ) {

// вызывается в начале работы и по приходу строки из приемника.

// выбор следующей посылаемой приемнику команды

  if ( CommonError )
    return FALSE; // на всякий случай
  
  if ( tV.f > CurTab.LastFreq )
    return FALSE; // на всякий случай
    
  if ( ScanMode == ScanInit ) {    
    ScanMode = ScanSetFreq;   // начало сканирования    
    tV.f = CurTab.FirstFreq;
    return TRUE;
  }
  
  if ( ScanMode == ScanSetFreq ) {
    ScanMode = ScanGetVFreq;
    EmptyTV = TRUE;
    
    return TRUE;
  }
  
  if ( ScanMode == ScanGetVFreq ) { 
    
    // выбор новой частоты для сканирования
    
    /*
    if ( tV.v != 0 )
      if ( (EmptyTV == TRUE) or ( ptVv != tV.v ) ) {
        // повторяем пока не установится
        EmptyTV = FALSE;
        ptVv = tV.v; 
        return TRUE; 
      }
    */
    
    if (tV.v == 0) {
      tSmallFreq = tV.f;
      NextFV( hWnd );

      if ( tV.f >= CurTab.LastFreq ) {
        tV.f = CurTab.LastFreq+1;
        ScanMode = ScanEnd;
        return FALSE;
      }  
        
      tV.f += BigStep;
      ScanMode = ScanSetFreq;
        
      if (tV.f > CurTab.LastFreq) 
        tV.f = CurTab.LastFreq;
    }
    else {
      if ( tV.f > tSmallFreq ) {
        PrevBig = tV;
        tV.f = tSmallFreq;
      }  
      else  
        NextFV( hWnd );
        
      if ( tV.f >= CurTab.LastFreq ) {
        tV.f = CurTab.LastFreq+1;
        ScanMode = ScanEnd;
        return FALSE;
      }  
        
      tV.f += SmallStep;
      
      if ( tV.f == PrevBig.f ) {
        tV = PrevBig;
        NextFV( hWnd );
        tV.f += SmallStep;
      }  
      
      ScanMode = ScanSetFreq;
        
      if (tV.f > CurTab.LastFreq) 
        tV.f = CurTab.LastFreq;

      tSmallFreq = tV.f;
      return TRUE;
    }
  }    
}


VOID CommSendStr( HWND hWnd ) {
  
  if ( CommonError )
    return;
    
  switch ( ScanMode ) {
    case ScanSetFreq :
      if ( commStrPtr != 0 ) {
        // можно еще попробовать повторить предыдущую команду
        // несколько раз
        CommonError = TRUE;
        return;
      }
      break;
    case ScanGetVFreq :
      if ( commStrPtr != 1 ) {
        // можно еще попробовать повторить предыдущую команду
        // несколько раз
        Message( hWnd, " comm err len ");
        CommonError = TRUE;
        return;
      }
      if ( commStr[0] == '%' ) 
        tV.v = 0;
      else {
        if ( (commStr[0] < 'A') || (commStr[0] > ('A'+15))) {
          Message( hWnd, " comm err char ");
          commStr[1] = 0;
          Message( hWnd, commStr);
          tV.v = 0;
          CommonError = TRUE;
        }  
        else 
          tV.v = commStr[0]-'A'+1;
      }
      break;
  }  
  
  if ( ScanStep( hWnd ) ) {
    switch ( ScanMode ) {
    
      case ScanSetFreq :
        clearCommStr();
        wsprintf ( commStr, "%ld.%04ld\r\n", tV.f/10000, tV.f%10000 );
        WriteComm( npTTYInfo.idComDev, commStr, strlen(commStr) ) ;
        break;

      case ScanGetVFreq :
        clearCommStr();
        WriteComm( npTTYInfo.idComDev, "y\r\n", 2 ) ;
        break;  
    }
  }
}

