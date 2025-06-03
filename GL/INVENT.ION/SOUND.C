#include <windows.h>
#include <mmsystem.h>
#include "sound.h"


WAVEBUFF    wBuff[mBuff];
HWAVEOUT    hWaveOut    = NULL;  // OutPut Device
HWAVEIN     hWaveIn     = NULL;  // Input Device
PCMWAVEFORMAT    PcmWaveFormat;  // Wave Format
HANDLE          hFormat = NULL;  // for Open Device


UINT  checkOk( HWND hWnd, UINT resultCode, UINT ioc )
{
  if ( resultCode ) 
    OutErrorMessage( hWnd, resultCode, ioc );
  return resultCode;
}

UINT checkNom( HWND hWnd, int nom ) {
  if (( nom < 0 ) ||( nom >= mBuff )) {
    messageWin( hWnd," Err Nom "); 
    return FALSE;
  }  
  return TRUE;
}  

//                  =====================                  
// =================      S O U N D      ================= 
//                  =====================                  


// ================= Err Code => String. ================= 


VOID        OutErrorMessage(HWND hWnd, UINT ErrCode, UINT ioc)
{
  char      str[MAXERRORLENGTH];
  
  if (ioc == IO_IN) {
    if (waveInGetErrorText(ErrCode,str,MAXERRORLENGTH)) {
      str[0] = '?';
      str[1] = 0;
    }
  }
  else if (ioc == IO_OUT) {
    if (waveOutGetErrorText(ErrCode,str,MAXERRORLENGTH)) {
      str[0] = '?';
      str[1] = 0;
    } 
  }
  else {
    messageWin( hWnd," Strange parameter. ");
    if (waveInGetErrorText(ErrCode,str,MAXERRORLENGTH)) {
      str[0] = '?';
      str[1] = 0;
    }
  } 
  messageWin( hWnd, str);
} 

// ==================== Prepare IN =======================

BOOL	prepareIn( HWND hWnd, int nom )
{
  HPSTR	lpData = NULL;

  if ( checkNom(hWnd, nom) == FALSE )
    return FALSE;
    
  if ( wBuff[nom].hWaveData == NULL ) {
    messageWin(hWnd, " err io prepare Out. ");
    return FALSE;
  }

  lpData = GlobalLock(wBuff[nom].hWaveData);
  
  if ( !lpData ) {
    messageWin(hWnd, "Failed to lock memory for data chunk.");
    return FALSE;
  }

  wBuff[nom].lpWaveHdr->lpData = lpData;

  if ( (wBuff[nom].io != IO_EMPTY) ||
       (wBuff[nom].lpWaveHdr == NULL) ||  
       (hWaveIn == NULL) ) {
    messageWin(hWnd, " err io prepare In. ");
    return FALSE;
  }
  if(waveInPrepareHeader(hWaveIn, wBuff[nom].lpWaveHdr, sizeof(WAVEHDR))) {
    messageWin(hWnd, " err code prepare In. ");
    return FALSE;
  }      
  wBuff[nom].io = IO_PREP_IN;
  return TRUE;
}

// ==================== Prepare OUT =======================

BOOL prepareOut( HWND hWnd, int nom )
{
HPSTR	lpData = NULL;

  if ( checkNom(hWnd,nom) == FALSE )
    return FALSE;

  if ( wBuff[nom].hWaveData == NULL ) {
    messageWin(hWnd, " err io prepare Out. ");
    return FALSE;
  }

  lpData = GlobalLock(wBuff[nom].hWaveData);
  
  if ( !lpData ) {
    messageWin(hWnd, "Failed to lock memory for data chunk.");
    return FALSE;
  }
        
  /* Set up WAVEHDR structure and prepare it to be written to wave device. */
    
  wBuff[nom].lpWaveHdr->lpData = lpData;

  if ( (wBuff[nom].io != IO_EMPTY) ||
       (wBuff[nom].lpWaveHdr == NULL) ||  
       (hWaveOut == NULL) ) {
    messageWin(hWnd, " err io prepare Out. ");
    return FALSE;
  }
  
  if(waveOutPrepareHeader(hWaveOut, wBuff[nom].lpWaveHdr, sizeof(WAVEHDR))) {
    messageWin(hWnd, " err code prepare In. ");
    return FALSE;
  }      
  
  wBuff[nom].io = IO_PREP_OUT;
  
  return TRUE;
}

// ==================== UnPrepare OUT =======================

UINT unprepareOut( HWND hWnd, int nom )
{
  UINT okCode = TRUE;
  
  if ( checkNom(hWnd,nom) == FALSE )
    return FALSE;

  if ( (wBuff[nom].io != IO_PREP_OUT) ||
       (wBuff[nom].lpWaveHdr == NULL) ||
       (hWaveOut == NULL) ) {
    messageWin(hWnd, " err io unprepare Out. ");
    return FALSE;
  }
  if (waveOutUnprepareHeader( hWaveOut, wBuff[nom].lpWaveHdr,
                                          sizeof(WAVEHDR))) {
    messageWin(hWnd, " err code unprepare Out. ");
    okCode = FALSE;
  }

  if ( (wBuff[nom].hWaveData == NULL)||
       (wBuff[nom].hWaveHdr  == NULL) ) {
    messageWin(hWnd, "Clear Buffers : Empty.");
    return FALSE;
  }
      
  GlobalUnlock( wBuff[nom].hWaveData );

  wBuff[nom].lpWaveHdr->lpData = NULL;
  
  wBuff[nom].io = IO_EMPTY;
  
  return okCode;
}

// ==================== UnPrepare IN =======================

UINT unprepareIn( HWND hWnd, int nom )
{
  UINT okCode = TRUE;
  
  if ( checkNom(hWnd, nom) == FALSE )
    return FALSE;

  if ( (wBuff[nom].io != IO_PREP_IN) ||
       (wBuff[nom].lpWaveHdr == NULL) ||  
       (hWaveIn == NULL) ) {
    messageWin(hWnd, " err io unprepare In. ");
    return FALSE;
  }
  if (waveInUnprepareHeader( hWaveIn, wBuff[nom].lpWaveHdr,
                                          sizeof(WAVEHDR))) {
    messageWin(hWnd, " err code unprepare In. ");
    okCode = FALSE;
  }                                       

  if ( (wBuff[nom].hWaveData == NULL) || 
      (wBuff[nom].hWaveHdr  == NULL) ) {
    messageWin(hWnd, "Clear Buffers : Empty.");
    return FALSE;
  }
      
  GlobalUnlock( wBuff[nom].hWaveData );

  wBuff[nom].lpWaveHdr->lpData = NULL;

  wBuff[nom].io = IO_EMPTY;
  return okCode;
}

// ============== Open Wave Device. ====================

UINT	OpenDevice( HWND hWnd, UINT ioc )
{
    PCMWAVEFORMAT   *pFormat = NULL;    
    UINT		    OkCode;
    char            *errMessage;
    
    pFormat = (PCMWAVEFORMAT *) LocalLock(hFormat);
    if (!pFormat) {
        errMessage = "Failed to lock memory for format chunk.";
        goto ErrExit;
    }
    
    // pFormat->wf = PcmWaveFormat.wf;
    // pFormat->wBitsPerSample = PcmWaveFormat.wBitsPerSample;

    // Input
    
    if ( ioc == IO_IN ) {
      OkCode = waveInOpen( NULL, (UINT)WAVE_MAPPER, 
        (LPWAVEFORMAT)pFormat, NULL, 0L, (DWORD)WAVE_FORMAT_QUERY );

      if ( OkCode ) {    
        errMessage = "The waveform device can't record this format.";
        goto ErrExit;
      }
    
      OkCode = waveInOpen( (LPHWAVEIN)&hWaveIn, (UINT)WAVE_MAPPER,
        (LPWAVEFORMAT)pFormat, (UINT)hWnd, 0L,
        (DWORD)CALLBACK_WINDOW);

      if ( OkCode == MMSYSERR_ALLOCATED ) {
        errMessage = "The waveform device already allocated.";
        goto ErrExit;
      } 
    
      if ( OkCode ) {
        OutErrorMessage( hWnd, OkCode,IO_IN );
 		errMessage = "Failed to open waveform record device.";
		goto ErrExit;
      }
    }
    else if ( ioc == IO_OUT ) { // Output
    
      if (waveOutOpen(&hWaveOut, (UINT)WAVE_MAPPER, (LPWAVEFORMAT)pFormat, NULL, 0L,
            (DWORD)WAVE_FORMAT_QUERY)) {
        errMessage = "The waveform device can't play this format.";
        goto ErrExit;
      }

      /* Open a waveform output device. */

      if (waveOutOpen((LPHWAVEOUT)&hWaveOut, (UINT)WAVE_MAPPER,
          (LPWAVEFORMAT)pFormat, (UINT)hWnd, 0L, (DWORD)CALLBACK_WINDOW)) {
        errMessage = "Failed to open waveform output device.";
        goto ErrExit;
      }
    } 
    else {
 		errMessage = " Err in OpenDevice io parameter. ";
		goto ErrExit;
    }
    
    LocalUnlock( hFormat );
    //LocalFree( hFormat );    

    return TRUE;
    
  ErrExit :
  
    messageWin(hWnd, errMessage);

    if ( hFormat ) {
      if ( pFormat )
        LocalUnlock( hFormat );
      //LocalFree( hFormat );
    }
    return FALSE;
}

// =============== Close Wave Device, check, clear ptr. =====

VOID	CloseDevice( HWND hWnd, UINT ioc )
{
    if ( ioc == IO_IN ) {
      if ( hWaveIn ) {
        checkOk(hWnd, waveInClose(hWaveIn),IO_IN);
        hWaveIn = NULL;
      }
    }
    else if ( ioc == IO_OUT ) {
      if ( hWaveOut ) {
        checkOk(hWnd, waveOutClose(hWaveOut),IO_OUT);
        hWaveOut = NULL;
      }
    }
    else {
 	  messageWin(hWnd, " Err in CloseDevice io parameter. ");
    }
}

// ============= Initial values (NULL's). ===============

VOID        InitBuffers( HWND hWnd )
{
  UINT i;
  PCMWAVEFORMAT   *pFormat = NULL;    
  
  PcmWaveFormat.wf.wFormatTag = WAVE_FORMAT_PCM;
  PcmWaveFormat.wf.nChannels = N_CHANNEL;
  PcmWaveFormat.wf.nSamplesPerSec = N_FREQ;
  PcmWaveFormat.wf.nAvgBytesPerSec = N_FREQ*N_CHANNEL*N_SAMPLE_BYTE;
  PcmWaveFormat.wf.nBlockAlign = N_CHANNEL*N_SAMPLE_BYTE;
  PcmWaveFormat.wBitsPerSample = 8*N_SAMPLE_BYTE;

  for ( i = 0; i < mBuff; i++ ) {
    wBuff[i].lpWaveHdr = NULL;
    wBuff[i].hWaveHdr = NULL;
    wBuff[i].hWaveData = NULL;
    wBuff[i].io = IO_NULL;
  }

    // for Open Device
    
  hFormat = LocalAlloc(LMEM_MOVEABLE, sizeof(PCMWAVEFORMAT));
    
  if (!hFormat) {
    messageWin(hWnd, "Out of memory.");
    return;
  }  
    
  pFormat = (PCMWAVEFORMAT *) LocalLock(hFormat);
  if (!pFormat) {
    messageWin(hWnd, "Failed to lock memory for format chunk.");
    return;
  }
    
  pFormat->wf = PcmWaveFormat.wf;
  pFormat->wBitsPerSample = PcmWaveFormat.wBitsPerSample;
  
  LocalUnlock(hFormat);  
}

// =================== Open Buffers ========================

UINT		OpenBuffer( HWND hWnd, int nom, DWORD dwDataSize, DWORD code )
{
    HPSTR           lpData    = NULL;
    char            *errMessage; 
                   
    if ( checkNom(hWnd, nom) == FALSE )
      return FALSE;

    wBuff[nom].hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                           (DWORD) sizeof(WAVEHDR));
    if (!wBuff[nom].hWaveHdr) {
      errMessage = "Not enough memory for Record header.";
      goto ErrExit;
    }
    
    wBuff[nom].lpWaveHdr = (LPWAVEHDR) GlobalLock(wBuff[nom].hWaveHdr);
    
    if ( !wBuff[nom].lpWaveHdr ) {
      errMessage = "Failed to lock memory for Record header.";
      goto ErrExit;
    }

    wBuff[nom].lpWaveHdr->lpData = NULL;
      
    wBuff[nom].hWaveData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwDataSize );
    if ( !wBuff[nom].hWaveData ) {
      errMessage = "Out of memory (Rec).";
      goto ErrExit;
    }
      
    /*
    HPSTR           lpData    = NULL;
    lpData = GlobalLock(wBuff[nom].hWaveData);
    if ( !lpData ) {
      errMessage = "Failed to lock memory for data chunk.";
      goto ErrExit;
    }
    */
        
    /* Set up WAVEHDR structure and prepare it to be written to wave device. */
    
    wBuff[nom].lpWaveHdr->lpData  = lpData;
    wBuff[nom].lpWaveHdr->dwBufferLength = dwDataSize;
    wBuff[nom].lpWaveHdr->dwFlags = 0L;
    wBuff[nom].lpWaveHdr->dwLoops = 0L;
    wBuff[nom].lpWaveHdr->dwUser  = code;
    wBuff[nom].io                 = IO_EMPTY;
    
    return TRUE;
    
  ErrExit :

    messageWin(hWnd, errMessage);
    CloseBuffers(hWnd);
    return FALSE;
}

// ==================== Close Buffers ========================
// (UnLock, Free).

UINT        CloseBuffers( HWND hWnd )
{
  UINT  i;
  char  *errMessage;
  
  
  if ( hFormat != NULL ) { // for Open Device
    LocalFree( hFormat ); 
    hFormat = NULL;
  }  

  if ( hWaveOut ) 
    waveOutReset( hWaveOut);

  if ( hWaveIn ) 
    waveInReset( hWaveIn);
  
  for ( i = 0; i < mBuff; i++ ) {
    
    /* Unprepare */
    
    if (wBuff[i].io == IO_PREP_IN ) unprepareIn(hWnd, i); 
    if (wBuff[i].io == IO_PREP_OUT ) unprepareOut(hWnd, i); 
    
    /* Lock(hWaveData) = lpWaveHdr->lpData */
    
    if ( GlobalFree( wBuff[i].hWaveData ) ) {
      errMessage = "Err GlobalFree Buffers.";
      goto ErrExit;
    }
    
    wBuff[i].hWaveData = NULL;
    wBuff[i].lpWaveHdr->lpData = NULL;
    
    /* Lock(hWaveHdr) = lpWaveHdr */
    
    GlobalUnlock( wBuff[i].hWaveHdr );

    if ( GlobalFree( wBuff[i].hWaveHdr ) ) {
      errMessage = "Err GlobalFree Buffers (2).";
      goto ErrExit;
    }
    
    wBuff[i].hWaveHdr = NULL;
    wBuff[i].lpWaveHdr = NULL;
    wBuff[i].io = IO_NULL;
  }

  CloseDevice(hWnd, IO_OUT);
  CloseDevice(hWnd, IO_IN);
  
  return TRUE;
  
  ErrExit : 
    messageWin(hWnd, errMessage);
  return FALSE;  
}

/*  ============================================================  */
/*  =                         Play Sound                       =  */
/*  ============================================================  */

UINT	PlayMySound(HWND hWnd,  int nom )
{
WORD    wResult;
        
    if ( checkNom(hWnd, nom) == FALSE )
      return FALSE;

    if (OpenDevice(hWnd, IO_OUT) == FALSE) return FALSE;
    
    if ( wBuff[nom].io != IO_EMPTY ) {
        messageWin(hWnd, "Buffers are not ready.");
        return FALSE;
    }

    wBuff[nom].lpWaveHdr->dwFlags = 0L;

    prepareOut(hWnd, nom);

    wResult = waveOutWrite(hWaveOut, wBuff[nom].lpWaveHdr, sizeof(WAVEHDR));
    
    if (wResult != 0) {
        unprepareOut(hWnd, nom);
        messageWin(hWnd, "Failed to write block to device");
        CloseDevice(hWnd, IO_OUT);
        return FALSE;
    }
    return TRUE;
}


/*  ============================================================  */
/*  =                         record                           =  */
/*  ============================================================  */


UINT RecordSoundOpen(HWND hWnd,  int nom ) 
{
    WORD            wResult;
    
    if ( checkNom(hWnd, nom) == FALSE )
      return FALSE;

    if (OpenDevice(hWnd, IO_IN) == FALSE) return FALSE;
    
    if ( wBuff[nom].io != IO_EMPTY ) {
        messageWin(hWnd, "Buffers are not ready.");
        return FALSE;
    }

    wBuff[nom].lpWaveHdr->dwFlags = 0L;
    prepareIn(hWnd, nom);
     
    wResult = waveInAddBuffer(hWaveIn, wBuff[nom].lpWaveHdr, sizeof(WAVEHDR));
    if (wResult != 0) {
        unprepareIn(hWnd, nom);
        messageWin(hWnd, "Failed to write block to Rec device");
        CloseDevice(hWnd, IO_IN);
        return FALSE;
    }
    
    return TRUE;

    wResult = waveInStart(hWaveIn);
    if (wResult != 0){
        unprepareIn(hWnd, nom);
        messageWin(hWnd, "Failed to start Rec device");
        CloseDevice(hWnd, IO_IN);
        return FALSE;
    }
    return TRUE;
}

/*  ============================================================  */
/*  =                         record                           =  */
/*  ============================================================  */


UINT RecordSound(HWND hWnd,  int nom ) 
{
    WORD            wResult;
    
    wResult = waveInStart(hWaveIn);
    if (wResult != 0){
        unprepareIn(hWnd, nom);
        messageWin(hWnd, "Failed to start Rec device");
        CloseDevice(hWnd, IO_IN);
        return FALSE;
    }
    return TRUE;
}

