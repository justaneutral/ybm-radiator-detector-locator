#include "cinc.h"
#include <stdlib.h>
#include "fcomm.h"
//#include "scnpaint.h"

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

TTYINFO npTTYInfo;

#define mCommStr 50

char commStr[mCommStr];

int  commStrPtr = 0; // указатель в commStr.

//BOOL scanError = FALSE;

enum { commStateReady, // принимаю все подряд.
       commStateR,     // жду N
       commStateN,     // жду >
       commStateP      // конец строки.
     } commState;


VOID clearCommStr( void ) {

  commStrPtr = 0;
  commState = commStateReady;

}  

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

LRESULT FAR CreateTTYInfo( HWND hWnd )
{
   // initialize TTY info structure

   COMDEV( npTTYInfo )        = 0 ;
   CONNECTED( npTTYInfo )     = FALSE ;
   //LOCALECHO( npTTYInfo )     = TRUE ;
   //AUTOWRAP( npTTYInfo )      = TRUE ;
   PORT( npTTYInfo )          = 1 ;
   //NEWLINE( npTTYInfo )       = 1 ;
   BAUDRATE( npTTYInfo )      = CBR_4800 ;
   BYTESIZE( npTTYInfo )      = 8 ;
   FLOWCTRL( npTTYInfo )      = 2 ; //FC_RTSCTS ;
   PARITY( npTTYInfo )        = NOPARITY ;
   STOPBITS( npTTYInfo )      = 2 ; // ONESTOPBIT ;
   XONXOFF( npTTYInfo )       = FALSE ;
   USECNRECEIVE( npTTYInfo )  = FALSE ; // TRUE ;
   DISPLAYERRORS( npTTYInfo ) = TRUE ;

   return ( (LRESULT) TRUE ) ;

} // end of CreateTTYInfo()



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

/*
BOOL FAR ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam )
{
   char       szError[ 10 ] ;
   int        nError, nLength ;
   BYTE       abIn[ MAXBLOCK + 1] ;
   COMSTAT    ComStat ;
   MSG        msg ;

   if (!USECNRECEIVE( npTTYInfo ))
   {
      // verify that it is a COMM event specified by our mask

      if (CN_EVENT & LOWORD( lParam ) != CN_EVENT)
         return ( FALSE ) ;

      // reset the event word so we are notified
      // when the next event occurs

      GetCommEventMask( COMDEV( npTTYInfo ), EV_RXCHAR ) ;

      // We loop here since it is highly likely that the buffer
      // can been filled while we are reading this block.  This
      // is especially true when operating at high baud rates
      // (e.g. >= 9600 baud).
      
      do
      {
         if (nLength = ReadCommBlock( hWnd, (LPSTR) abIn, MAXBLOCK )) {
           if (scanState == 1) scanState = 2;
           if (scanState == 3) { scanState = 0;
              if (nLength > 3) {
                //printW(hWnd,IDC_SCAN_TEXT, (LPSTR) abIn) ;
                scS.pV[scS.nxtV] = abIn[0]-'A';
                scS.nxtV++;
                scanRepCount++;
              }  
           }   
         }     
      }
      while (!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE) ||
             (nLength > 0)) ;
   }
   else
   {
      // verify that it is a receive event

      if (CN_RECEIVE & LOWORD( lParam ) != CN_RECEIVE)
         return ( FALSE ) ;

      do
      {
         if (nLength = ReadCommBlock( hWnd, (LPSTR) abIn, MAXBLOCK )) {
            //printW(hWnd,IDC_SCAN_TEXT, (LPSTR) abIn) ;
         }
         if (nError = GetCommError( COMDEV( npTTYInfo ), &ComStat ))
         {
            if (DISPLAYERRORS( npTTYInfo )) {
               wsprintf( szError, "<CE-%d>", nError ) ;
               MessageBox( hWnd, szError, " ", MB_ICONEXCLAMATION ) ;
            }
         }
      }
      while ((!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE )) ||
            (ComStat.cbInQue >= MAXBLOCK)) ;
   }

   return ( TRUE ) ;

} // end of ProcessCOMMNotification()
*/

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

int FAR ReadCommBlock( HWND hWnd, LPSTR lpszBlock, int nMaxLength )
{
   char       szError[ 10 ] ;
   int        nLength, nError ;

   nLength = ReadComm( COMDEV( npTTYInfo ), lpszBlock, nMaxLength ) ;

   if (nLength < 0)
   {
      nLength *= -1 ;
      while (nError = GetCommError( COMDEV( npTTYInfo ), NULL ))
      {
         if (DISPLAYERRORS( npTTYInfo ))
         {
            wsprintf( szError, "<CE-%d>", nError ) ;
            MessageBox( hWnd, szError, " ",
                   MB_ICONEXCLAMATION ) ;
         }
      }
   }

   return ( nLength ) ;

} // end of ReadCommBlock()


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


BOOL FAR ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam )
{
   char       szError[ 10 ] ;
   int        nError, nLength, i ;
   BYTE       abIn[ MAXBLOCK + 1] ;
   COMSTAT    ComStat ;

   if (!USECNRECEIVE( npTTYInfo ))
   {
      // verify that it is a COMM event specified by our mask

      if (CN_EVENT & LOWORD( lParam ) != CN_EVENT)
         return ( FALSE ) ;

      // reset the event word so we are notified
      // when the next event occurs

      GetCommEventMask( COMDEV( npTTYInfo ), EV_RXCHAR ) ;

      // We loop here since it is highly likely that the buffer
      // can been filled while we are reading this block.  This
      // is especially true when operating at high baud rates
      // (e.g. >= 9600 baud).
      
      do
      {
         if ( ( nLength = ReadCommBlock( hWnd, (LPSTR) abIn, MAXBLOCK )) > 0 ) {
           for (i = 0; i < nLength; i++) {
             if ( (char)(abIn[i]) == '\r' ) {
               if ( commState == commStateReady )
                 commState = commStateR;
               else  
                 MessageBox( hWnd, "err comm state R", " ", MB_ICONEXCLAMATION ) ;
             
             }
             else if ( (char)(abIn[i]) == '\n' ) {
               if ( commState == commStateR ) 
                 commState = commStateN;
               else  
                 MessageBox( hWnd, "err comm state N", " ", MB_ICONEXCLAMATION ) ;
             
             }
             else if ( (char)(abIn[i]) == '>' ) {
               if ( commState == commStateN ) {
                 commState = commStateP;
               }  
               else  
                 MessageBox( hWnd, "err comm state P", " ", MB_ICONEXCLAMATION ) ;
             
             }
             else if ( commState == commStateReady ) {
                 if ( commStrPtr < mCommStr ) {
                   commStrPtr++;
                   commStr[commStrPtr-1] = (char)(abIn[i]);
                 }
                 else {
                   MessageBox( hWnd, "comm str overflow", " ", MB_ICONEXCLAMATION ) ;
                   commStrPtr = 0;
                 }
             }
             else
               MessageBox( hWnd, "strange getcomm", " ", MB_ICONEXCLAMATION ) ;
           }
         }     
      }
      while (/*!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE) || */(nLength > 0)) ;
   }
   else
   {
      // verify that it is a receive event

      if (CN_RECEIVE & LOWORD( lParam ) != CN_RECEIVE)
         return ( FALSE ) ;

      do
      {
         if ((nLength = ReadCommBlock( hWnd, (LPSTR) abIn, MAXBLOCK ))!= 0) {
           
            //printW(hWnd,IDC_check_TEXT, (LPSTR) abIn) ;
         }
         
         if ((nError = GetCommError( COMDEV( npTTYInfo ), &ComStat )) != 0 )
         {
            if (DISPLAYERRORS( npTTYInfo )) {
               wsprintf( szError, "<CE-%d>", nError ) ;
               MessageBox( hWnd, szError, " ", MB_ICONEXCLAMATION ) ;
            }
         }
      }
      while (/*(!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE )) ||*/
            (ComStat.cbInQue >= MAXBLOCK)) ;
   }

   if ( commState == commStateP )
     return ( TRUE ) ;
   else
     return ( FALSE ) ;

} // end of ProcessCOMMNotification()


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

BOOL FAR SetupConnection( HWND hWnd )
{
   BOOL       fRetVal ;
   BYTE       bSet ;
   DCB        dcb ;

   GetCommState( COMDEV( npTTYInfo ), &dcb ) ;

   dcb.BaudRate = BAUDRATE( npTTYInfo ) ;
   dcb.ByteSize = BYTESIZE( npTTYInfo ) ;
   dcb.Parity = PARITY( npTTYInfo ) ;
   dcb.StopBits = STOPBITS( npTTYInfo ) ;

   // setup hardware flow control

   bSet = (BYTE) ((FLOWCTRL( npTTYInfo ) & FC_DTRDSR) != 0) ;
   dcb.fOutxDsrFlow = dcb.fDtrflow = bSet ;
   dcb.DsrTimeout = (bSet) ? 30 : 0 ;

   bSet = (BYTE) ((FLOWCTRL( npTTYInfo ) & FC_RTSCTS) != 0) ;
   dcb.fOutxCtsFlow = dcb.fRtsflow = bSet ;
   dcb.CtsTimeout = (bSet) ? 30 : 0 ;

   // setup software flow control

   bSet = (BYTE) ((FLOWCTRL( npTTYInfo ) & FC_XONXOFF) != 0) ;

   dcb.fInX = dcb.fOutX = bSet ;
   dcb.XonChar = ASCII_XON ;
   dcb.XoffChar = ASCII_XOFF ;
   dcb.XonLim = 100 ;
   dcb.XoffLim = 100 ;

   // other various settings

   dcb.fBinary = TRUE ;
   dcb.fParity = TRUE ;
   dcb.fRtsDisable = FALSE ;
   dcb.fDtrDisable = FALSE ;

   fRetVal = !(SetCommState( &dcb ) < 0) ;

   return ( fRetVal ) ;

} // end of SetupConnection()


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

BOOL FAR OpenConnection( HWND hWnd )
{
   char       szPort[ 10 ]; //, szTemp[ 10 ] ;
   BOOL       fRetVal ;
   HCURSOR    hOldCursor, hWaitCursor ;
   // HMENU      hMenu ;

   hWaitCursor = LoadCursor( NULL, IDC_WAIT ) ;
   hOldCursor = SetCursor( hWaitCursor ) ;

   wsprintf( szPort, "COM%d", PORT( npTTYInfo ) ) ;

   // open COMM device

   if ((COMDEV( npTTYInfo ) = OpenComm( szPort, RXQUEUE, TXQUEUE )) < 0)
      return ( FALSE ) ;

   fRetVal = SetupConnection( hWnd ) ;

   if (fRetVal)
   {
      CONNECTED( npTTYInfo ) = TRUE ;

      // set up notifications from COMM.DRV

      if (!USECNRECEIVE( npTTYInfo ))
      {
         // In this case we really are only using the notifications
         // for the received characters - it could be expanded to
         // cover the changes in CD or other status lines.

         SetCommEventMask( COMDEV( npTTYInfo ), EV_RXCHAR ) ;

         // Enable notifications for events only.

         // NB:  This method does not use the specific
         // in/out queue triggers.

         EnableCommNotification( COMDEV( npTTYInfo ), hWnd, -1, -1 ) ;
      }
      else
      {
         // Enable notification for CN_RECEIVE events.

         EnableCommNotification( COMDEV( npTTYInfo ), hWnd, MAXBLOCK, -1 ) ;
      }

      // assert DTR

      EscapeCommFunction( COMDEV( npTTYInfo ), SETDTR ) ;
   }
   else
   {
      CONNECTED( npTTYInfo ) = FALSE ;
      CloseComm( COMDEV( npTTYInfo ) ) ;
   }

   // restore cursor
   SetCursor( hOldCursor ) ;

   return ( fRetVal ) ;

} // end of OpenConnection()

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


BOOL FAR CloseConnection( HWND hWnd )
{

   // Disable event notification.  Using a NULL hWnd tells
   // the COMM.DRV to disable future notifications.

   EnableCommNotification( COMDEV( npTTYInfo ), NULL, -1, -1 ) ;

   // drop DTR

   EscapeCommFunction( COMDEV( npTTYInfo ), CLRDTR ) ;

   // close comm connection

   CloseComm( COMDEV( npTTYInfo ) ) ;
   CONNECTED( npTTYInfo ) = FALSE ;

   return ( TRUE ) ;

} // end of CloseConnection()

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

BOOL FAR WriteCommByte( HWND hWnd, BYTE bByte )
{
   WriteComm( COMDEV( npTTYInfo ), (LPSTR) &bByte, 1 ) ;

   return ( TRUE ) ;

} // end of WriteCommByte()

BOOL FAR ConnectionOn( HWND hWnd, BOOL St )  //    Setup Connection / Close
{
  if ( St ) {
    clearCommStr();
         
    CreateTTYInfo( hWnd ) ;
    if (!OpenConnection( hWnd )) {
      Message( hWnd, "Connection failed." );
      return FALSE;
      CommonError = TRUE;
    }  
  }
  else {
    if (CONNECTED( npTTYInfo ))
      CloseConnection( hWnd );  
  }    
  return TRUE;  
}