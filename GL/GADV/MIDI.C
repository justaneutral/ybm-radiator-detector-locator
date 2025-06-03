#include "cinc.h"
#include "string.h"

#define  mBuffer  128

char sBuffer[mBuffer];


VOID SendString(HWND Window, LPCSTR s) {

long  dwErr;

  if ( CommonError )
    return;
  
  dwErr = mciSendString(s, NULL, 0, Window);
  
  CommonError = (dwErr != 0);

  if ( CommonError ) {
    mciGetErrorString(dwErr, sBuffer, mBuffer);
    Message(Window,sBuffer);
  }
} /* SendString */


BOOL midiOn(HWND Window, LPCSTR filename, LPCSTR channel, BOOL St) {

  if ( CommonError )
    return FALSE;
    
  if ( St ) { 
    strcpy(sBuffer, "open sequencer!");
    strcat(sBuffer, filename);
    strcat(sBuffer, " alias ");
    strcat(sBuffer, channel);
    SendString(Window, sBuffer);
    return CommonError;
  }
  else {
    strcpy(sBuffer, "close ");
    strcat(sBuffer, channel);
    SendString(Window, sBuffer);
    return CommonError;
  }  
} /* midiClose */


VOID midiRePlay(HWND Window, LPCSTR channel) {

  if ( CommonError )
    return;

  strcpy(sBuffer, "seek ");
  strcat(sBuffer, channel);
  strcat(sBuffer, " to start wait");
  SendString(Window, sBuffer);

  strcpy(sBuffer, "play ");
  strcat(sBuffer, channel);
  SendString(Window, sBuffer);
} /* midiPlay */


VOID midiReset(HWND Window, LPCSTR channel) {

  if ( CommonError )
    return;

  strcpy(sBuffer, "seek ");
  strcat(sBuffer, channel);
  strcat(sBuffer, " to start wait");
  SendString(Window, sBuffer);

} /* midiPlay */

VOID midiPlay(HWND Window, LPCSTR channel) {

  if ( CommonError )
    return;

  //strcpy(sBuffer, "seek ");
  //strcat(sBuffer, channel);
  //strcat(sBuffer, " to start wait");
  //SendString(Window, sBuffer);

  strcpy(sBuffer, "play ");
  strcat(sBuffer, channel);
  SendString(Window, sBuffer);
} /* midiPlay */



VOID midiStop(HWND Window, LPCSTR channel) {

  if ( CommonError )
    return;

  strcpy(sBuffer, "stop ");
  strcat(sBuffer, channel);
  SendString(Window, sBuffer);
} /* midiStop */


short midiStatus(HWND Window, LPCSTR channel) {

static char rBuffer[mBuffer];
long  dwErr;

  if ( CommonError )
    return 2;

  /*
    1 - plaing,
    3 - stopped.
  */

  strcpy(sBuffer, "status ");
  strcat(sBuffer, channel);
  strcat(sBuffer, " mode");
  dwErr = mciSendString(sBuffer, rBuffer, mBuffer, Window);

  if ( dwErr == 263 )
    return 0;
  else
  if ( strcmp(rBuffer,"playing") == 0 )
    return 1;
  else
  if ( strcmp(rBuffer,"not ready") == 0 )
    return 2;
  else
  if ( strcmp(rBuffer,"paused") == 0 )
    return 2;
  else
  if ( strcmp(rBuffer,"seeking") == 0 )
    return 2;
  else
  if ( strcmp(rBuffer,"stopped") == 0 )
    return 3;
  else {
    Message(Window, "Strange MIDI Status");
    CommonError = TRUE;
    return 2;
  }
} /* midiStatus */
