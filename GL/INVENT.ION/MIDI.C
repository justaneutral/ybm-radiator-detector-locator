
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include 	"sound.h"

static char    aszBuffer[BUFFER_LENGTH];

DWORD SendString(HWND hWnd, char *s)
{
  char    aszReturn[BUFFER_LENGTH];
  DWORD   dwErr;

  strcpy(aszBuffer,s);
  dwErr = mciSendString(aszBuffer, aszReturn, sizeof(aszReturn), hWnd);
  mciGetErrorString(dwErr, aszBuffer, BUFFER_LENGTH);

  return dwErr;
}

VOID	midiInit( HWND hWnd, char *filename, char *channel ) {
  DWORD l;
  char  buff[256];
  
  wsprintf(buff,"open sequencer!%s alias %s",filename,channel);
  l = SendString(hWnd, buff);
}

VOID	midiPlay( HWND hWnd, char *channel ) {
  DWORD l;
  char  buff[256];
  
  wsprintf(buff,"seek %s to start wait",channel);
  l = SendString(hWnd, buff);
  
  wsprintf(buff,"play %s",channel);
  l = SendString(hWnd, buff);

}

VOID	midiClose( HWND hWnd, char *channel ) {
  DWORD l;
  char  buff[256];
  
  wsprintf(buff,"close %s",channel);
  l = SendString(hWnd, buff);
  
}

UINT	midiStatus( HWND hWnd, char *channel ) {

// 1 - plaing,
// 3 - stopped.

  char    aszReturn[BUFFER_LENGTH];
  DWORD   dwErr;
  //char    buff[256];
  
  wsprintf(aszBuffer,"status %s mode",channel);
  
  //l = SendString(hWnd, buff);
  //strcpy(aszBuffer,buff);
    
  dwErr = mciSendString(aszBuffer, aszReturn, sizeof(aszReturn), hWnd);

  if ( dwErr == 263 ) 
    return 0;
  if ( strcmp(aszReturn,"playing") == 0 )   
    return 1;
  if ( strcmp(aszReturn,"not ready") == 0 )   
    return 2;
  if ( strcmp(aszReturn,"paused") == 0 )   
    return 2;
  if ( strcmp(aszReturn,"seeking") == 0 )   
    return 2;
  if ( strcmp(aszReturn,"stopped") == 0 ) 
    return 3;
   
  
  messageWin(hWnd, " Strange MIDI Status ");
   
  return 2;
}

