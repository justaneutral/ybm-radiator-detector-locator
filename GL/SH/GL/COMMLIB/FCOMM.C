#include "cinc.h"
#include <stdlib.h>
#include "fcomm.h"


TTYINFO npTTYInfo;

#define mCommStr      50

char commStr[mCommStr]; // сюда записывается получаемая строка

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

   npTTYInfo.idComDev        = 0 ;
   npTTYInfo.fConnected     = FALSE ;
   npTTYInfo.bPort          = 1 ;
   npTTYInfo.wBaudRate      = CBR_4800 ;
   npTTYInfo.bByteSize      = 8 ;
   npTTYInfo.bFlowCtrl      = 2 ; //FC_RTSCTS ;
   npTTYInfo.bParity        = NOPARITY ;
   npTTYInfo.bStopBits      = 2 ; // ONESTOPBIT ;
   npTTYInfo.fXonXoff       = FALSE ;
   npTTYInfo.fUseCNReceive  = FALSE ; // TRUE ;
   npTTYInfo.fDisplayErrors = TRUE ;

   return ( (LRESULT) TRUE ) ;

} // end of CreateTTYInfo()


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
   
   if ( error )
     return 0;

   nLength = ReadComm( npTTYInfo.idComDev, lpszBlock, nMaxLength ) ;

   if (nLength < 0)
   {
      nLength *= -1 ;
      while (nError = GetCommError( npTTYInfo.idComDev, NULL )) {
         if ( npTTYInfo.fDisplayErrors ) {
           wsprintf( szError, "<CE-%d>", nError ) ;
           errormessage( hWnd, szError ) ;
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

   if ( error ) 
     return FALSE;

   if (!( npTTYInfo.fUseCNReceive ))
   {
      // verify that it is a COMM event specified by our mask

      if (CN_EVENT & LOWORD( lParam ) != CN_EVENT)
         return ( FALSE ) ;

      // reset the event word so we are notified
      // when the next event occurs

      GetCommEventMask( npTTYInfo.idComDev, EV_RXCHAR ) ;

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
               else { 
                 errormessage( hWnd, "Unexpected comm chars (CR)");
                 return FALSE;
               }             
             }
             else if ( (char)(abIn[i]) == '\n' ) {
               if ( commState == commStateR ) 
                 commState = commStateN;
               else {
                 errormessage( hWnd, "Unexpected comm chars (NL)") ;
                 return FALSE;
               }  
             }
             else if ( (char)(abIn[i]) == '>' ) {
               if ( commState == commStateN ) {
                 commState = commStateP;
               }  
               else { 
                 errormessage( hWnd, "Unexpected comm chars (>)" ) ;
                 return FALSE;
               }  
             }
             else if ( commState == commStateReady ) {
                 if ( commStrPtr < mCommStr ) {
                   commStrPtr++;
                   commStr[commStrPtr-1] = (char)(abIn[i]);
                 }
                 else {
                   errormessage( hWnd, "Unexpected comm chars (buffer overflow)." );
                   commStrPtr = 0;
                   return FALSE;
                 }
             }
             else {
               errormessage( hWnd, "Unexpected comm chars" );
               return FALSE;
             }  
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
         }
         
         if ((nError = GetCommError( npTTYInfo.idComDev, &ComStat )) != 0 )
         {
            if ( npTTYInfo.fDisplayErrors) {
               wsprintf( szError, "<CE-%d>", nError ) ;
               errormessage( hWnd, szError ) ;
               return FALSE;
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

   if ( error ) 
     return FALSE;

   GetCommState( npTTYInfo.idComDev, &dcb ) ;

   dcb.BaudRate = npTTYInfo.wBaudRate ;
   dcb.ByteSize = npTTYInfo.bByteSize;
   dcb.Parity = npTTYInfo.bParity;
   dcb.StopBits = npTTYInfo.bStopBits;

   // setup hardware flow control

   bSet = (BYTE) ( (( npTTYInfo.bFlowCtrl ) & FC_DTRDSR) != 0) ;
   dcb.fOutxDsrFlow = dcb.fDtrflow = bSet ;
   dcb.DsrTimeout = (bSet) ? 30 : 0 ;

   bSet = (BYTE) ( (( npTTYInfo.bFlowCtrl ) & FC_RTSCTS) != 0) ;
   dcb.fOutxCtsFlow = dcb.fRtsflow = bSet ;
   dcb.CtsTimeout = (bSet) ? 30 : 0 ;

   // setup software flow control

   bSet = (BYTE) ( ( ( npTTYInfo.bFlowCtrl ) & FC_XONXOFF) != 0) ;

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
   char       szPort[ 10 ];
   BOOL       fRetVal ;
   HCURSOR    hOldCursor, hWaitCursor ;

   if ( error ) 
     return FALSE;

   hWaitCursor = LoadCursor( NULL, IDC_WAIT ) ;
   hOldCursor = SetCursor( hWaitCursor ) ;

   wsprintf( szPort, "COM%d", npTTYInfo.bPort ) ;

   // open COMM device

   if ((npTTYInfo.idComDev = OpenComm( szPort, RXQUEUE, TXQUEUE )) < 0)
      return ( FALSE ) ;

   fRetVal = SetupConnection( hWnd ) ;

   if (fRetVal)
   {
      npTTYInfo.fConnected = TRUE ;

      // set up notifications from COMM.DRV

      if (!( npTTYInfo.fUseCNReceive ))
      {
         // In this case we really are only using the notifications
         // for the received characters - it could be expanded to
         // cover the changes in CD or other status lines.

         SetCommEventMask( npTTYInfo.idComDev, EV_RXCHAR ) ;

         // Enable notifications for events only.

         // NB:  This method does not use the specific
         // in/out queue triggers.

         EnableCommNotification( npTTYInfo.idComDev, hWnd, -1, -1 ) ;
      }
      else
      {
         // Enable notification for CN_RECEIVE events.

         EnableCommNotification( npTTYInfo.idComDev, hWnd, MAXBLOCK, -1 ) ;
      }

      // assert DTR

      EscapeCommFunction( npTTYInfo.idComDev, SETDTR ) ;
   }
   else
   {
      npTTYInfo.fConnected = FALSE ;
      CloseComm( npTTYInfo.idComDev ) ;
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
  if ( error ) 
    return FALSE;

   // Disable event notification.  Using a NULL hWnd tells
   // the COMM.DRV to disable future notifications.

   EnableCommNotification( npTTYInfo.idComDev, NULL, -1, -1 ) ;

   // drop DTR

   EscapeCommFunction( npTTYInfo.idComDev, CLRDTR ) ;

   // close comm connection

   CloseComm( npTTYInfo.idComDev ) ;
   npTTYInfo.fConnected = FALSE ;

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
  if ( error ) 
    return FALSE;

   WriteComm( npTTYInfo.idComDev, (LPSTR) &bByte, 1 ) ;

   return ( TRUE ) ;

} // end of WriteCommByte()

BOOL FAR ConnectionOn( HWND hWnd, BOOL St )  //    Setup Connection / Close
{
  if ( error ) 
    return FALSE;
  
  if ( St ) {
    clearCommStr();
         
    CreateTTYInfo( hWnd ) ;
    if (!OpenConnection( hWnd )) {
      errormessage( hWnd, "Connection failed." );
      return FALSE;
    }  
  }
  else {
    if ( npTTYInfo.fConnected )
      CloseConnection( hWnd );  
  }    
  return TRUE;  
}