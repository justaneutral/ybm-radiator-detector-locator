#include "cinc.h"
//#include <malloc.h>
#include <string.h>


//BOOL CommonError = FALSE;
                                         
HWAVEIN hWaveInp = NULL;
HWAVEOUT hWaveOut = NULL;
PCMWAVEFORMAT wFormat;

lpWaveBuffer WaveBufferList = NULL;


VOID CopyWave( WaveBuffer *pb ) 
{
  OneSample *ab;
  short i,m;
  
  m = GetMemDC(sl)->m;

  ab = (OneSample *)(pb->WaveHdr.lpData);
  
  for ( i = 0; i < m; i++ ) 
    sr[i] = ab[i].leftSample;

  for ( i = 0; i < m; i++ ) 
    sl[i] = ab[i].rightSample;

}


BOOL checkOk(HWND Window, short ErrCode, BOOL inpDev) {

char str[MAXERRORLENGTH];

  if (ErrCode == 0 )
    return TRUE;

  CommonError = TRUE;

  if (inpDev ) { /* input */
    if (waveInGetErrorText(ErrCode,str,MAXERRORLENGTH) != 0 ) 
      strcpy( str, "?");
  }
  else { /* output */
    if (waveOutGetErrorText(ErrCode,str,MAXERRORLENGTH) != 0 ) 
      strcpy( str, "?");
  }
  Message( Window, str);
} /* checkOk */


VOID FillWaveFormat(PCMWAVEFORMAT *pwf, short sb_freq) {

short nOfBytes = 2; /* число байт в одом отсчете ( 1 или 2 ) */

  pwf->wf.wFormatTag = WAVE_FORMAT_PCM;
  pwf->wf.nChannels = 2;
  pwf->wf.nSamplesPerSec = 11025L*sb_freq;
  pwf->wf.nBlockAlign = (pwf->wf.nChannels)*nOfBytes;
  pwf->wf.nAvgBytesPerSec = (pwf->wf.nSamplesPerSec)*(pwf->wf.nBlockAlign);
  pwf->wBitsPerSample = 8*nOfBytes;

} /* FillWaveFormat */


/* ___________________ Prepare Inp ____________________ */


VOID prepareInp(HWND Window, lpWaveBuffer bf) {

  if ( CommonError )
    return;

  CommonError = TRUE;

  if (bf == NULL ) return;

  if ( (bf->WaveHdr.lpData == NULL) or (hWaveInp == 0) or
     (bf->hWaveData == 0) or (bf->state != bfIdle) ) {
    Message( Window, "Internal Error(2)" );
    return;
  }

  if (waveInPrepareHeader( hWaveInp, &(bf->WaveHdr),
                sizeof(WAVEHDR)) != 0 ) {
    Message(Window,"Error In Prepare Input Buffer");
    return;
  }

  bf->state = bfPrepInp;
  
  CommonError = FALSE;
  
} /* Prepare Inp */


/* ____________________ Prepare Out _____________________ */


VOID prepareOut(HWND Window, lpWaveBuffer bf) {
  if ( CommonError )
    return;

  CommonError = TRUE;

  if ( bf == NULL ) return;

  if ( (bf->WaveHdr.lpData == NULL) or (hWaveOut == NULL) or
     (bf->hWaveData == NULL) or (bf->state != bfIdle) ) {
    Message(Window, "Internal Error(4)");
    return;
  }

  if (waveOutPrepareHeader(hWaveOut, &(bf->WaveHdr),
                sizeof(WAVEHDR)) != 0 ) {
    Message(Window,"Error In Prepare Output Buffer");
    return;
  }

  bf->state = bfPrepOut;
  CommonError = FALSE;
} /* Prepare Out */


/* _________________ UnPrepare Out __________________ */


VOID unprepareOut(HWND Window, lpWaveBuffer bf) {
  if ( CommonError )
    return;

  CommonError = TRUE;

  if ( bf == NULL ) return;

  if ( (bf->WaveHdr.lpData == NULL) or (hWaveOut == NULL) or
     (bf->hWaveData == NULL) or (bf->state != bfPrepOut) ) {
    Message(Window, "Internal Error(5)");
    return;
  }

  if ( waveOutUnprepareHeader(hWaveOut, &(bf->WaveHdr),
                         sizeof(WAVEHDR)) != 0 ) {
    Message(Window,"Error in Unprepare Output Buffer");
    return;
  }

  bf->state = bfIdle;
  CommonError = FALSE;
} /* Unprepare Out */


/* ________________ UnPrepare In ________________ */


VOID unprepareInp(HWND Window, lpWaveBuffer bf) {
  if ( CommonError )
    return;

  CommonError = TRUE;

  if ( bf == NULL ) return;

  if ( (bf->WaveHdr.lpData == NULL) or (hWaveInp == NULL) or
     (bf->hWaveData == NULL) or (bf->state != bfPrepInp) ) {
    Message(Window, "Internal Error(7)");
    return;
  }

  if ( waveInUnprepareHeader(hWaveInp, &(bf->WaveHdr),
                         sizeof(WAVEHDR)) != 0 ) {
    Message(Window,"Error in Unprepare Input Buffer");
    return;
  }

  bf->state = bfIdle;
  CommonError = FALSE;
} /* unprepare Inp */


/* __________________ Out Wave ___________________ */


VOID WaveOut(HWND Window, lpWaveBuffer bf) {
  if ( CommonError )
    return;

  if ( bf == NULL ) return;

  if ( (bf->WaveHdr.lpData == NULL) or (hWaveOut != NULL) or
     (bf->hWaveData == 0) or (bf->state != bfIdle) ) {
    Message(Window, "Internal Error(8)");
    CommonError = TRUE;
    return;
  }

  OpenDevice(Window,FALSE);

  if ( CommonError )
    return;

  bf->WaveHdr.dwFlags = 0;

  prepareOut(Window, bf);

  if ( CommonError )
    return;

  if ( waveOutWrite(hWaveOut,
              &(bf->WaveHdr), sizeof(WAVEHDR)) != 0 ) {
    unprepareOut(Window, bf);
    Message(Window, "Error in OutWave");
    CloseDevice(Window, FALSE);
    CommonError = TRUE;
    return;
  }

  /*bf->state = bfOut;*/
} /* Wave Out */


/* ___________________ Wave Inp ____________________ */


VOID WaveInp(HWND Window, lpWaveBuffer bf) {
  if ( CommonError )
    return;

  if ( bf == NULL ) return;

  if ( (bf->WaveHdr.lpData == NULL) or (hWaveInp != NULL) or
     (bf->hWaveData == NULL) or (bf->state != bfIdle) ) {
    Message(Window, "Internal Error.1");
    CommonError = TRUE;
    return;
  }

  OpenDevice(Window,TRUE);

  if ( CommonError )
    return;

  bf->WaveHdr.dwFlags = 0;

  prepareInp(Window, bf);

  if ( CommonError )
    return;

  if ( waveInAddBuffer(hWaveInp, &(bf->WaveHdr),
                      sizeof(WAVEHDR)) != 0 ) {
    unprepareInp(Window, bf);
    Message(Window, "Error in InpWave");
    CloseDevice(Window, TRUE);
    CommonError = TRUE;
    return;
  }

  if (waveInStart(hWaveInp) != 0 ) {
    unprepareInp(Window, bf);
    Message(Window, "Error in InpWave");
    CloseDevice(Window, TRUE);
    CommonError = TRUE;
    return;
  }

  /*bf->state = bfInp;*/
} /* Wave Inp */


/* ________________ Open Wave Device ________________ */


VOID OpenDevice(HWND Window, BOOL inpDev) {

  short retCode;

  if ( CommonError )
    return;

  CommonError = TRUE;

  if ( inpDev ) { /* Input Device */
    hWaveInp = NULL;
    retCode = waveInOpen( NULL, (UINT)WAVE_MAPPER, 
        (LPWAVEFORMAT)(&wFormat), NULL, 0L, (DWORD)WAVE_FORMAT_QUERY );
    
    if (retCode != 0 ) {
      Message(Window,"The waveform device can""t record this format.");
      return;
    }
    
    retCode = waveInOpen( (LPHWAVEIN)&hWaveInp, (UINT)WAVE_MAPPER,
        (LPWAVEFORMAT)(&wFormat), (UINT)Window, 0L,
        (DWORD)CALLBACK_WINDOW);

    if ( retCode == MMSYSERR_ALLOCATED ) {
      Message(Window, "The waveform device already allocated.");
      return;
    }

    if ( retCode != 0 ) {
      Message(Window, "Open Wave Device Error.");
      checkOk(Window,retCode,TRUE);
      return;
    }

    if ( hWaveInp == 0 ) {
      Message(Window,"Ptr Error");
      return;
    }
  }
  else { /* Output Device */
    hWaveOut = NULL;
    if (waveOutOpen(&hWaveOut, (UINT)WAVE_MAPPER, (LPWAVEFORMAT)(&wFormat), NULL, 0L,
            (DWORD)WAVE_FORMAT_QUERY)) {
      Message(Window,"The waveform device can't record this format.");
      return;
    }

      /* Open a waveform output device. */

    if (waveOutOpen((LPHWAVEOUT)&hWaveOut, (UINT)WAVE_MAPPER,
        (LPWAVEFORMAT)(&wFormat), (UINT)Window, 0L, (DWORD)CALLBACK_WINDOW)) {
      Message(Window,"Failed to open waveform output device.");
      return;
    }
  }
  CommonError = FALSE;
}


/* ________________ Close Wave Device _______________ */


VOID CloseDevice( HWND Window, BOOL inpDev ) {

  if (CommonError )
    return;

  CommonError = TRUE;
  if (inpDev ) { /* input */
    if (hWaveInp != NULL ) {
      if ( !checkOk(Window, waveInClose(hWaveInp), TRUE) )
        return;
      hWaveInp = NULL;
    }
  }
  else { /* output */
    if (hWaveOut != NULL ) {
      if ( !checkOk(Window, waveOutClose(hWaveOut), FALSE) )
        return;
      hWaveOut = NULL;
    }
  }

  CommonError = FALSE;
}


/* ___________________ Open Buffers ____________________ */


VOID  OpenBuffer(HWND Window, long dataSize,
                    short bfCode, lpWaveBuffer bf) {

  if ( CommonError )
    return;

  CommonError = TRUE;
  
  bf->hWaveData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dataSize);
  if ( bf->hWaveData == NULL ) {
    Message(Window,"Not enough memory.");
    return;
  }

  bf->WaveHdr.lpData = GlobalLock(bf->hWaveData);

  if (bf->WaveHdr.lpData == NULL ) {
    Message(Window,"Failed to lock memory for data chunk.");
    return;
  }

  /* Set up WAVEHDR structure and prepare it to be written to wave device. */

  bf->WaveHdr.dwBufferLength = dataSize;
  bf->WaveHdr.dwFlags = 0;
  bf->WaveHdr.dwLoops = 0;
  bf->WaveHdr.dwUser  = bfCode;
  bf->state            = bfIdle;
  bf->next             = WaveBufferList;
  WaveBufferList = bf;

  CommonError = FALSE;
}


/* ____________________ Open AdcDac ____________________ */


BOOL WaveOn(HWND Window, BOOL St, short sb_freq) {
/* Инициализация */
  lpWaveBuffer bf;

  if ( CommonError )
    return FALSE;
  
  if ( St ) {
    WaveBufferList = NULL;
    hWaveInp = NULL;
    hWaveOut = NULL;

    FillWaveFormat((PCMWAVEFORMAT *)(&wFormat),sb_freq);
  }  
  else {
  
  CommonError = TRUE;

  if ( hWaveOut != 0 ) {
    waveOutReset(hWaveOut);
    hWaveOut = 0;
  }

  if ( hWaveInp != 0 ) {
    waveInReset(hWaveInp);
    hWaveInp = 0;
  }

  while ( WaveBufferList != NULL ) {
    bf = WaveBufferList;
    /* Unprepare */

    if ( bf->state == bfPrepInp ) unprepareInp(Window, bf);
    else if ( bf->state == bfPrepOut ) unprepareOut(Window, bf);

    if ( GlobalUnlock(bf->hWaveData) != 0 ) {
      Message(Window, "Error in Unlock Wave Buffer.");
      return FALSE;
    }

    if ( GlobalFree(bf->hWaveData) != 0 ) {
      Message(Window, "Error in GlobalFree Buffers.");
      return FALSE;
    }

    bf->hWaveData = 0;
    bf->state = bfNull;
    WaveBufferList = bf->next;
    // FreeMem(b,sizeOf(WaveBuffer));
  }

  CloseDevice(Window, TRUE);
  CloseDevice(Window, FALSE);
    
  CommonError = FALSE;
  return TRUE;
  }
}

