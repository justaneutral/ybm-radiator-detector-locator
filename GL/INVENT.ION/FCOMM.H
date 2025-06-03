
//#define WIN31        // this is a Windows 3.1 application
//#define USECOMM      // yes, we need the COMM API
//#define STRICT       // be bold!

#include <windows.h>
#include <commdlg.h>
#include <string.h>

//#include "version.h"
//#include "resource.h"

// constant definitions

#define GWW_NPTTYINFO       0
#define ABOUTDLG_USEBITMAP  1

#define ATOM_TTYINFO       0x100

// terminal size

#define MAXROWS         25
#define MAXCOLS         80

#define MAXBLOCK        80

#define MAXLEN_TEMPSTR  81

#define RXQUEUE         4096
#define TXQUEUE         4096

// cursor states

#define CS_HIDE         0x00
#define CS_SHOW         0x01

// Flow control flags

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

// ascii definitions

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

// data structures

typedef struct tagTTYINFO
{
   int     idComDev ;
   BYTE    bPort;
   //, abScreen[ MAXROWS * MAXCOLS ] ;
   BOOL    fConnected, fXonXoff, //fLocalEcho, //fNewLine, fAutoWrap,
           fUseCNReceive, fDisplayErrors;
   BYTE    bByteSize, bFlowCtrl, bParity, bStopBits ;
   WORD    wBaudRate;
} TTYINFO;

#define COMDEV( x ) (x.idComDev)
#define PORT( x )   (x.bPort)
#define SCREEN( x ) (x.abScreen)
#define CONNECTED( x ) (x.fConnected)
#define XONXOFF( x ) (x.fXonXoff)
//#define LOCALECHO( x ) (x.fLocalEcho)
//#define NEWLINE( x ) (x.fNewLine)
//#define AUTOWRAP( x ) (x.fAutoWrap)
#define BYTESIZE( x ) (x.bByteSize)
#define FLOWCTRL( x ) (x.bFlowCtrl)
#define PARITY( x ) (x.bParity)
#define STOPBITS( x ) (x.bStopBits)
#define BAUDRATE( x ) (x.wBaudRate)

#define USECNRECEIVE( x ) (x.fUseCNReceive)
#define DISPLAYERRORS( x ) (x.fDisplayErrors)


/*
WORD     gawBaudTable[] = { CBR_110,
                            CBR_300,
                            CBR_600,
                            CBR_1200,
                            CBR_2400,
                            CBR_4800,
                            CBR_9600,
                            CBR_14400,
                            CBR_19200,
                            CBR_38400,
                            CBR_56000,
                            CBR_128000,
                            CBR_256000   } ;

WORD     gawParityTable[] = { NOPARITY,
                              EVENPARITY,
                              ODDPARITY,
                              MARKPARITY,
                              SPACEPARITY } ;

WORD     gawStopBitsTable[] = { ONESTOPBIT,
                                ONE5STOPBITS,
                                TWOSTOPBITS } ;

*/
// function prototypes (private)

//BOOL FAR InitApplication( HANDLE ) ;
//HWND FAR InitInstance( HANDLE, int ) ;
LRESULT FAR CreateTTYInfo( HWND ) ;
//BOOL FAR DestroyTTYInfo( HWND ) ;
//BOOL FAR ProcessTTYCharacter( HWND, BYTE ) ;
//BOOL FAR WriteTTYBlock( HWND, LPSTR, int ) ;
int FAR ReadCommBlock( HWND, LPSTR, int ) ;
BOOL FAR WriteCommByte( HWND, BYTE ) ;
BOOL FAR OpenConnection( HWND ) ;
BOOL FAR SetupConnection( HWND ) ;
BOOL FAR CloseConnection( HWND ) ;
//BOOL FAR ProcessCOMMNotification( HWND, WORD, LONG ) ;
//VOID FAR GoModalDialogBoxParam( HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM ) ;
//BOOL FAR SettingsDlgTerm( HWND ) ;

//---------------------------------------------------------------------------
//  End of First Part of File (tty.h).
//---------------------------------------------------------------------------


//==============================================================

extern TTYINFO npTTYInfo;

extern char commStr[50];

int myDialogBox( HWND hWnd, LPCSTR lpszTemplate, FARPROC lpDlgProc);

//---------------------------------------------------------------------------
//  LRESULT FAR CreateTTYInfo( HWND hWnd )
//
//  Description:
//     Creates the tty information structure and sets
//     menu option availability.  Returns -1 if unsuccessful.
//
//  Parameters:
//     HWND  hWnd
//        Handle to main window.
//
//---------------------------------------------------------------------------

LRESULT FAR CreateTTYInfo( HWND hWnd );
   // initialize TTY info structure

//---------------------------------------------------------------------------
//  BOOL FAR ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam )
//
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


BOOL FAR ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam );

//---------------------------------------------------------------------------
//  BOOL FAR OpenConnection( HWND hWnd )
//
//  Description:
//     Opens communication port specified in the TTYINFO struct.
//     It also sets the CommState and notifies the window via
//     the fConnected flag in the TTYINFO struct.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//---------------------------------------------------------------------------

BOOL FAR OpenConnection( HWND hWnd );

//---------------------------------------------------------------------------
//  BOOL FAR SetupConnection( HWND hWnd )
//
//  Description:
//     This routines sets up the DCB based on settings in the
//     TTY info structure and performs a SetCommState().
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//---------------------------------------------------------------------------

BOOL FAR SetupConnection( HWND hWnd );

//---------------------------------------------------------------------------
//  BOOL FAR CloseConnection( HWND hWnd )
//
//  Description:
//     Closes the connection to the port.  Resets the connect flag
//     in the TTYINFO struct.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//---------------------------------------------------------------------------


BOOL FAR CloseConnection( HWND hWnd );

//---------------------------------------------------------------------------
//  int FAR ReadCommBlock( HWND hWnd, LPSTR lpszBlock, int nMaxLength )
//
//  Description:
//     Reads a block from the COM port and stuffs it into
//     the provided block.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//     LPSTR lpszBlock
//        block used for storage
//
//     int nMaxLength
//        max length of block to read
//
//---------------------------------------------------------------------------

int FAR ReadCommBlock( HWND hWnd, LPSTR lpszBlock, int nMaxLength );

//---------------------------------------------------------------------------
//  BOOL FAR WriteCommByte( HWND hWnd, BYTE bByte )
//
//  Description:
//     Writes a byte to the COM port specified in the associated
//     TTY info structure.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//     BYTE bByte
//        byte to write to port
//
//---------------------------------------------------------------------------

BOOL FAR WriteCommByte( HWND hWnd, BYTE bByte );
//===================================================================

int		printW(HWND hDlg, UINT ne, char *s, ...);

