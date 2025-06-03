#include <windows.h>
#include <commdlg.h>
#include <string.h>


#define MAXBLOCK        80

// Flow control flags

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#define RXQUEUE         4096
#define TXQUEUE         4096


typedef struct tagTTYINFO
{
   int     idComDev ;
   BYTE    bPort;
   BOOL    fConnected, fXonXoff, 
           fUseCNReceive, fDisplayErrors;
   BYTE    bByteSize, bFlowCtrl, bParity, bStopBits ;
   WORD    wBaudRate;
} TTYINFO;

//==============================================================

extern TTYINFO npTTYInfo;

extern char commStr[50];

extern int commStrPtr;

//extern BOOL ScanError;

BOOL FAR ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam );

//---------------------------------------------------------------------------
//  Description:
//     Processes the WM_COMMNOTIFY message from the COMM.DRV.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//     WORD wParam
//        specifes the device (nCid)
//
//     LONG lParam
//        LOWORD contains event trigger
//        HIWORD is NULL
//
//---------------------------------------------------------------------------


VOID clearCommStr( void );

BOOL FAR ConnectionOn( HWND hWnd, BOOL St );  //    Setup Connection / Close



int		printW(HWND hDlg, UINT ne, char *s, ...);

int myDialogBox( HWND hWnd, LPCSTR lpszTemplate, FARPROC lpDlgProc);
