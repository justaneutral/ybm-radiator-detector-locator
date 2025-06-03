#include "cinc.h"
#include <string.h>

HWAVEIN hWaveInp = NULL;
HWAVEOUT hWaveOut = NULL;
PCMWAVEFORMAT wFormat;

static lpWaveBuffer waveBufferList = NULL; // для закрытия буферов
static lpWaveBuffer waveOnBufferList = NULL; // для MM_WOM_DONE

static short nOfBuffers = 0;
static UINT waveRetCode = 0;

static struct SoundBlasterTag {
  short
   nOfBytes, // число байт в одом отсчете ( 1 (8 bit) или 2 (16 bit)
   nOfChannels, // 1 or 2
   frequency; //  1 or 2 or 4 (11025Hz, 22050Hz, 44100Hz)
  
  waveDirection 
   direction; // waveInput / waveOutput
} soundBlaster = { 2,2,1, waveNull };

static BOOL buffersOnMode = FALSE; // для проверки 
                     // правильного порядка waveOn и bufferOn


static BOOL notOk(HWND hWnd, UINT ErrCode) {

  char str[MAXERRORLENGTH];

  waveRetCode = ErrCode;
  
  if (ErrCode == 0 )
    return FALSE;

  if ( soundBlaster.direction == waveInp ) { // input 
    if (waveInGetErrorText(ErrCode,str,MAXERRORLENGTH) != 0 ) 
      strcpy( str, "AdcDac Error (Input)");
  }
  else { // output 
    if (waveOutGetErrorText(ErrCode,str,MAXERRORLENGTH) != 0 ) 
      strcpy( str, "AdcDac Error (Output)");
  }
  
  errormessage( hWnd, str);

  if ( soundBlaster.direction == waveNull )
    message(hWnd, " Sound Blaster direction error ");
  return TRUE;
} 


static VOID FillWaveFormat(PCMWAVEFORMAT *pwf) {
  pwf->wf.wFormatTag = WAVE_FORMAT_PCM;
  pwf->wf.nChannels = soundBlaster.nOfChannels;
  pwf->wf.nSamplesPerSec = (11025L*soundBlaster.frequency);
  pwf->wf.nBlockAlign = (pwf->wf.nChannels)*soundBlaster.nOfBytes;
  pwf->wf.nAvgBytesPerSec = (pwf->wf.nSamplesPerSec)*(pwf->wf.nBlockAlign);
  pwf->wBitsPerSample = 8*soundBlaster.nOfBytes;
}


static VOID prepareBuffer(HWND hWnd, lpWaveBuffer bf) {

  if ( error )
    return;

  if ( bf == NULL ) return;
  
  switch ( soundBlaster.direction ) {
  
  case waveInp: // ____________________ Inp ______________________

#ifdef _DEBUG

    if ( (bf->WaveHdr.lpData == NULL) or (hWaveInp == NULL) or
       (bf->hWaveData == NULL) or (bf->state != bfUnPrep) ) {
      errormessage(hWnd, "Unprepare Input Buffer Data Internal Error (1)");
      return;
    }

#endif

    if ( notOk( hWnd, waveInPrepareHeader( hWaveInp,
            &(bf->WaveHdr), sizeof(WAVEHDR))) ) 
      return;

    bf->state = bfPrepInp;

    break;
    
  case waveOut: // ____________________ Out ______________________

#ifdef _DEBUG

    if ( (bf->WaveHdr.lpData == NULL) or (hWaveOut == NULL) or
       (bf->hWaveData == NULL) or (bf->state != bfUnPrep) ) {
      errormessage(hWnd, "Unprepare Output Buffer Data Internal Error (2)");
      return;
    }

#endif

    if ( notOk(hWnd,waveOutPrepareHeader(hWaveOut,
           &(bf->WaveHdr), sizeof(WAVEHDR)))) 
      return;

    bf->state = bfPrepOut;
    break;
    
  case waveNull:

  default :

    errormessage(hWnd,"Prepare Wave Buffer Error (d)");

    return;
  }    
}


static VOID unprepareBuffer (HWND hWnd, lpWaveBuffer bf) {
  
  if ( error )
    return;

#ifdef _DEBUG

  if ( bf == NULL ) return;

#endif

  switch ( soundBlaster.direction ) {
  
  case waveInp: // ____________________ Inp ______________________

#ifdef _DEBUG

    if ( (bf->WaveHdr.lpData == NULL) or (hWaveInp == NULL) or
       (bf->hWaveData == NULL) or (bf->state == bfUnPrep) ) {
      errormessage(hWnd, "Unprepare Input Buffer Data Internal Error (3)");
      return;
    }

#endif

    if ( notOk(hWnd, waveInUnprepareHeader(hWaveInp,
       &(bf->WaveHdr), sizeof(WAVEHDR))) ) 
      return;
      
    break;  

  case waveOut:  // ____________________ Out ______________________

#ifdef _DEBUG

    if ( (bf->WaveHdr.lpData == NULL) or (hWaveOut == NULL) or
       (bf->hWaveData == NULL) or (bf->state == bfUnPrep) ) {
      errormessage(hWnd, "Unprepare Output Buffer Data Internal Error (4)");
      return;
    }

#endif

    if ( notOk ( hWnd, waveOutUnprepareHeader(hWaveOut,
           &(bf->WaveHdr), sizeof(WAVEHDR)))) 
      return;
    
    break;
    
  case waveNull:

  default :
    errormessage(hWnd,"Unprepare Wave Buffer Error (d)");
    return;
  }    
  
  bf->WaveHdr.dwFlags = 0;
  bf->WaveHdr.dwLoops = 0;
  bf->state = bfUnPrep;
}


static VOID openDevice ( HWND hWnd ) {

  if ( error )
    return;

  switch ( soundBlaster.direction ) {
  
  case waveInp: // __________________ Inp ____________________
  
    if ( hWaveInp != NULL ) {
      errormessage(hWnd, "Open Wave Device Error");
      return;
    }  

    if ( notOk ( hWnd, waveInOpen( NULL, (UINT)WAVE_MAPPER, 
        (LPWAVEFORMAT)(&wFormat), NULL, 0L, (DWORD)WAVE_FORMAT_QUERY ) ) ) {

      message(hWnd,"The waveform device can""t record this format.");
      return;
    }
    
    if ( notOk ( hWnd, waveInOpen( (LPHWAVEIN)&hWaveInp,
         (UINT)WAVE_MAPPER, (LPWAVEFORMAT)(&wFormat),
         (UINT)hWnd, 0L, (DWORD)CALLBACK_WINDOW) ) )

      if ( waveRetCode == MMSYSERR_ALLOCATED ) 
        message(hWnd, "The waveform device already allocated.");

      return;

    if ( hWaveInp == 0 ) {
      errormessage(hWnd,"Ptr Error");
      return;
    }
    break;
    
  case waveOut: // _____________ Out _______________
  
    if ( hWaveOut != NULL ) {
      errormessage(hWnd, "Open Wave Device Error");
      return;
    }  

    if ( notOk ( hWnd, waveOutOpen(&hWaveOut, 
       (UINT)WAVE_MAPPER, (LPWAVEFORMAT)(&wFormat), NULL, 0L,
            (DWORD)WAVE_FORMAT_QUERY)) ) {
            
      message(hWnd,"The waveform device can""t record this format.");
      return;
    }

    if ( notOk ( hWnd, waveOutOpen((LPHWAVEOUT)&hWaveOut, 
        (UINT)WAVE_MAPPER,    (LPWAVEFORMAT)(&wFormat), 
         (UINT)hWnd, 0L, (DWORD)CALLBACK_WINDOW))) {

      if ( waveRetCode == MMSYSERR_ALLOCATED ) 
        message(hWnd, "The waveform device already allocated.");
         
      return;
    }
    break;
      
  case waveNull:
  default :
    errormessage(hWnd,"Open Sound Blaster Error (d).");
    return;
  }
}

// ________________ Close Device _______________ 

static VOID closeDevice( HWND hWnd ) {

  if ( error )
    return;
    
  if ( waveOnBufferList != NULL ) {
    // errormessage ( hWnd, "Close Wave Device Error");
    return;
  }    

  switch ( soundBlaster.direction ) {
  
  case waveInp: // __________________ Inp ____________________
  
    if (hWaveInp != NULL ) {
      if ( notOk (hWnd, waveInReset(hWaveInp)) )
        return;

      if ( notOk( hWnd, waveInClose(hWaveInp)) )
        return;

      hWaveInp = NULL;
    }
    
    break;
    
  case waveOut: // _____________ Out _______________

    if ( hWaveOut != NULL ) {

      if ( notOk(hWnd,waveOutReset(hWaveOut)) )
        return;

      if ( notOk( hWnd, waveOutClose(hWaveOut)) )
        return;

      hWaveOut = NULL;
    }  
    break;
      
  case waveNull:
  
  default :
    errormessage(hWnd,"Close Sound Blaster Error (d).");
    return;
  }
}


VOID waveStart(HWND hWnd) {

  if ( error )
    return;
    
#ifdef _DEBUG

  if ( waveOnBufferList == NULL ) {
    errormessage(hWnd, "Wave Start Error");
    return;
  }  

  if ( soundBlaster.direction != waveInp ) {
    errormessage(hWnd,"Error in Wave Start (d)");
    return;
  }
    
#endif

  if ( notOk(hWnd,waveInStart(hWaveInp)) ) 
    return;
} 


VOID waveAddBuffer(HWND hWnd, lpWaveBuffer bf) {
  lpWaveBuffer blst; // для закрытия буферов

  if ( error )
    return;

#ifdef _DEBUG

    if ( bf == NULL ) 
      return;
    
    if ( soundBlaster.direction == waveNull ) {
      errormessage(hWnd, " Wave Buffer Add Error");
      return;
    }  

#endif

    if ( soundBlaster.direction == waveInp ) {

#ifdef _DEBUG

      if ( (bf->WaveHdr.lpData == NULL) or (hWaveInp == NULL) or
        (bf->hWaveData == NULL) or (bf->state == bfUnPrep) ) {
        errormessage(hWnd, "Wave Input Add Buffer Data Internal Error (5)");
        return;
      }

#endif

    if ( notOk ( hWnd, waveInAddBuffer(hWaveInp,
      &(bf->WaveHdr), sizeof(WAVEHDR))) ) 
      return;
    bf->state = bfInp;
  }  
  else {

#ifdef _DEBUG

    if ( (bf->WaveHdr.lpData == NULL) or (hWaveOut == NULL) or
       (bf->hWaveData == NULL) or (bf->state == bfUnPrep) ) {
      errormessage(hWnd, "Wave Output Add Buffer Data Internal Error (6)");
      return;
    }

#endif

    if ( notOk(hWnd, waveOutWrite(hWaveOut,
              &(bf->WaveHdr), sizeof(WAVEHDR))) )
      return;

    bf->state = bfOut;
  }

  bf->nextOn = NULL;
  blst = waveOnBufferList;
  
  if ( waveOnBufferList == NULL ) {
    waveOnBufferList	  = bf;
    return;
  }
  
  while ( blst->nextOn != NULL )
    blst = blst->nextOn;
    
  blst->nextOn 		  = bf;
} 


VOID  bufferOn(HWND hWnd, lpWaveBuffer bf, long dataSize,
                    short bfCode, BOOL St) {
  if ( error )
    return;
  
  if ( St != TRUE ) 
    return;
  
  if ( !buffersOnMode ) {
    errormessage(hWnd, "Wave On / Buffer On Order Error");
    return;
  }  
  
  bf->hWaveData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dataSize);
  if ( bf->hWaveData == NULL ) {
    errormessage(hWnd,"Not enough memory.");
    return;
  }

  bf->WaveHdr.lpData = GlobalLock(bf->hWaveData);

  if (bf->WaveHdr.lpData == NULL ) {
    errormessage(hWnd,"Failed to lock memory for data chunk.");
    return;
  }

  /* Set up WAVEHDR structure. */

  bf->WaveHdr.dwBufferLength = dataSize;
  bf->WaveHdr.dwFlags = 0;
  bf->WaveHdr.dwLoops = 0;
  bf->WaveHdr.dwUser  = 777+bfCode;
  bf->state           = bfUnPrep;
  bf->next 		      = waveBufferList;
  waveBufferList	  = bf;
  
  nOfBuffers++; // общее число буферов
  
  if ( soundBlaster.direction != waveNull )
    errormessage(hWnd," Alloc buffer error (d)");
}


VOID setDirection( HWND hWnd, waveDirection direction ) {
// Установка одного из режимов: АЦП / ЦАП / Ничего.

  lpWaveBuffer bf;

  if ( error )
    return;
    
  if ( waveOnBufferList != NULL )  {
    errormessage(hWnd, "Set Direction Error (1)");
    return;
  }  
    
  if ( soundBlaster.direction != waveNull ) {
  
    bf = waveBufferList;
  
    while ( bf != NULL ) {
      if ( bf->state != bfUnPrep )
        unprepareBuffer(hWnd, bf);
      bf = bf->next;
    }
    closeDevice(hWnd);
  }
  
  soundBlaster.direction = direction;
  
  if ( soundBlaster.direction != waveNull ) {
    openDevice(hWnd);
  
    bf = waveBufferList;
  
    while ( bf != NULL ) {
      prepareBuffer(hWnd,bf);
      bf = bf->next;
    }
  }  
}


short waveDone( HWND hWnd, LONG lParam ) {
// вызывается после MM_WIM_DONE и MM_WOM_DONE.
// изменяет состояние буфера
// возвращает номер буфера

  DWORD dwU;
  
  if ( waveOnBufferList == NULL ) {
    errormessage( hWnd, " Wave Input Done Error (2).");
    return 0;
  }
    
  dwU = ((struct wavehdr_tag far *)lParam)->dwUser;
    
  if ( waveOnBufferList->WaveHdr.dwUser != dwU ) {
    errormessage( hWnd, " Wave Done dwUser Error (3).");
    return 0;
  }

  switch ( soundBlaster.direction ) {
  
  case waveInp:
    if ( waveOnBufferList->state != bfInp ) {
      errormessage(hWnd, "MM_WIM_DONE buffer state error");
	  return 0;
    }
    waveOnBufferList->state = bfPrepInp;
    break;
    
  case waveOut:

    if ( waveOnBufferList->state != bfOut ) {
      errormessage(hWnd, "MM_WOM_DONE buffer state error");
	  return 0;
    }
    
    waveOnBufferList->state = bfPrepOut;
    break;

  default:
    errormessage(hWnd, "MM_WOM_DONE buffer state error (7)");
    return 0;
  }
  
  waveOnBufferList = waveOnBufferList->nextOn;

#ifdef _DEBUG

  if ( dwU < 777 ) {
    errormessage(hWnd, "MM_WIM_WOM_DONE Buffer Nom Error (4).");
    return 0;
  }
      
#endif

  dwU -= 777;

#ifdef _DEBUG

  if ( dwU >= (DWORD)nOfBuffers ) {
    errormessage(hWnd, "MM_WIM_WOM_DONE Buffer Nom Error (5).");
    return 0;
  }

#endif

  return (short)dwU;
}

/* ____________________ On / Off ____________________ */

VOID waveOn ( 
    HWND hWnd,
    short nnOfBytes,          // 1,2
    short nnOfChannels,       // 1,2
    short nfrequency,         // 1,2,4
    BOOL St) {
    
/* Инициализация */

  lpWaveBuffer bf;

  if ( error )
    return;
  
  if ( St ) {
    if ( buffersOnMode ) {
      errormessage(hWnd, " WaveOn / BufferOn Order Error ");
      return;
    }
    
    buffersOnMode = TRUE;  
    waveBufferList = NULL;
    waveOnBufferList = NULL;
    hWaveInp = NULL;
    hWaveOut = NULL;
    
    soundBlaster.direction = waveNull;
    soundBlaster.frequency = nfrequency;
    soundBlaster.nOfBytes = nnOfBytes;
    soundBlaster.nOfChannels = nnOfChannels;
    
    FillWaveFormat((PCMWAVEFORMAT *)(&wFormat));
  }  
  else {
    if ( !buffersOnMode ) {
      errormessage(hWnd, "Wave On / Buffer On Order Error");
      return;
    }
      
    if ( hWaveOut != 0 ) 
      if ( notOk ( hWnd, waveOutReset(hWaveOut) ) )
        return;

    if ( hWaveInp != 0 ) 
      if ( notOk( hWnd, waveInReset(hWaveInp) ) )
        return;

    while ( waveBufferList != NULL ) {
      bf = waveBufferList;

      if ( bf->state != bfUnPrep )
        unprepareBuffer(hWnd, bf);

      if ( GlobalUnlock(bf->hWaveData) != 0 ) {
        errormessage(hWnd, "Error in Unlock Wave Buffer.");
        return;
      }

      if ( GlobalFree(bf->hWaveData) != 0 ) {
        errormessage(hWnd, "Error in GlobalFree Buffers.");
        return;
      }

      bf->hWaveData = 0;
      bf->state = bfUnPrep;
      waveBufferList = bf->next;
    }

    buffersOnMode = FALSE;
    nOfBuffers = 0; // общее число буферов

    if ( soundBlaster.direction != waveNull )
      closeDevice(hWnd);
    
    return;
  }
}

