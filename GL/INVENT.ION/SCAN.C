#include <windows.h>
#include <mmsystem.h>

#include <stdlib.h>
#include "fcomm.h"
#include "scnpaint.h"
#include "resource.h"
#include "sound.h"

static BOOL scanMode = FALSE;
static int scanState = 0, scanRepCount = 0;

static PICTURE_TEXT_BUTTON_WINDOW scanWin;

VOID freqReport(HWND hWnd) {      // Text String
  long tf;
  
  tf = scS.f1+scS.fStep*scS.nxtV;
  printW(hWnd,IDC_WIN_TEXT,
  "From  %ld.%04ld mHz  (%ld.%04ld mHz)  to  %ld.%04ld mHz.  Step  %ld.%ld kHz.",
    (scS.f1/10000),(scS.f1%10000),
    (tf/10000),    (tf%10000),
    (scS.f2/10000),(scS.f2%10000),
    (scS.fStep/10),(scS.fStep%10)
    );
}

VOID scanClose(HWND hWnd) {       // Stop Scan.
  HMENU hMenu;
  
  hMenu = GetMenu(hWnd);
            
  EnableMenuItem( hMenu, IDM_SCAN_STOP,
       MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
  EnableMenuItem( hMenu, IDM_SCAN_START,
                   MF_ENABLED | MF_BYCOMMAND ) ;
  printW(hWnd,IDC_WIN_START, (LPSTR) "Start") ;
 
  scanMode = FALSE;

  repaintFreqScale(hWnd,FALSE);
  freqReport(hWnd);
  scanRepCount = 0;
}

int scanSendStep(HWND hWnd) {
  long tf;
  
  if (scanState == 0) {
  if ( ( scS.nxtV >= scS.nV ) || ( (scS.f1 < scS.f2) && (scS.fStep == 0))) {
    scanMode = FALSE;
    scanClose(hWnd);
    return 1;
  }
  
  tf = scS.f1+scS.fStep*scS.nxtV;
  
  if ( tf > scS.f2 ) {
    scanMode = FALSE;
    scanClose(hWnd);
    return 1;
  }
                                                                              
  wsprintf(commStr,"%ld.%04ld\r",(tf/10000),(tf%10000));
  WriteComm( COMDEV( npTTYInfo ), commStr, strlen(commStr) ) ;
  
  scanState = 1;
  return 0;  
  }
  if (scanState == 2) {
    WriteComm( COMDEV( npTTYInfo ), "y\r", 2 ) ;
    scanState = 3;
    return 0;
  }
  return 0;
}

BOOL FAR PASCAL __export CommandProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {
// Test Mode (Send String) Dialog.
    switch (msg) {
	  case WM_INITDIALOG:
	    return (TRUE);

	  case WM_COMMAND:
	    if ( wParam == IDCANCEL) {
	        EndDialog(hDlg, 2);
		  return (TRUE);
	    }
	    if (wParam == IDOK) {
          if (!GetWindowText( GetDlgItem(hDlg,IDC_COMMAND), (LPSTR)commStr, 15))
            EndDialog(hDlg, FALSE);
	 	  else
	        EndDialog(hDlg, TRUE);
		  return (TRUE);
	    }
	    break;
    }
    return (FALSE);
}

long roundCheck ( double f ) {
  return (long)f;
}  

BOOL FAR PASCAL __export ScanStartProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {
// Get Scan Parameters Dialog.

  long nf1,nf2,nfs;
  
  double f;
  
  char *s;

    switch (msg) {
	  case WM_INITDIALOG:
	    printW(hDlg,IDC_GET_F1, (LPSTR)"1") ;
	    printW(hDlg,IDC_GET_F2, (LPSTR)"2036") ;
	    printW(hDlg,IDC_GET_FS, (LPSTR)"100") ;
	    
	    return (TRUE);

	  case WM_COMMAND:
	    switch ( wParam ) {

	      case IDCANCEL:
            EndDialog(hDlg, 1);
            break;

	      case IDOK:
            if (!GetWindowText( GetDlgItem(hDlg,IDC_GET_F1), (LPSTR)commStr, 15)) {
              EndDialog(hDlg, 0);
              break;
            }  

            f = strtod(commStr,&s);
            
            if ( s[0] != NULL ) {
              MessageBox( hDlg, s, " ",MB_ICONEXCLAMATION ) ;
              return (FALSE);
            }

            nf1 = roundCheck(f*10000.0);

            if (!GetWindowText( GetDlgItem(hDlg,IDC_GET_F2), (LPSTR)commStr, 15)) {
              EndDialog(hDlg, 0);
              break;
            }  

            f = strtod(commStr,&s);
            
            if ( s[0] != NULL ) {
              MessageBox( hDlg, s, " ",MB_ICONEXCLAMATION ) ;
              return (FALSE);
            }

            nf2 = roundCheck(f*10000.0);

            if (!GetWindowText( GetDlgItem(hDlg,IDC_GET_FS), (LPSTR)commStr, 15)) {
              EndDialog(hDlg, 0);
              break;
            }
            
            f = strtod(commStr,&s);
            
            if ( s[0] != NULL ) {
              MessageBox( hDlg, s, " ",MB_ICONEXCLAMATION ) ;
              return (FALSE);
            }
          
            nfs = roundCheck(f*10.0);
            
            if ( nf1 > nf2 ) {
              MessageBox( hDlg, " f1 > f2 ", " ",MB_ICONEXCLAMATION ) ;
              return (FALSE);
            }
          
            if ( nfs < 1 ) {
              MessageBox( hDlg, " Error frequency step ", " ",MB_ICONEXCLAMATION ) ;
              return (FALSE);
            }

            if ( clearFreqPaint() == FALSE ) {
              EndDialog(hDlg, 0);
              break;
            }

            if ( initFreqData( nf1,nf2,nfs ) == FALSE ) {
              EndDialog(hDlg, 0);
              break;
            }
          
            EndDialog(hDlg, 2);
            return (TRUE);
            break;
        }
        break;
    }
    return (FALSE);
}

BOOL FAR PASCAL __export ScanTest(HWND hWnd, unsigned msg, WORD wParam, LONG lParam)
{   
  BYTE b;
  HMENU hMenu;
  int i;
  
  switch (msg) {
    case WM_COMMAND:
      switch (wParam) {
        case IDM_GOTO_CHECK:
          if (CONNECTED( npTTYInfo ))
            CloseConnection( hWnd ) ;
          EndDialog( hWnd, 3 );
          break;

        case IDM_GOTO_LOCATE:
          if (CONNECTED( npTTYInfo ))
            CloseConnection( hWnd ) ;
          EndDialog( hWnd, 4 );
          break;

        case IDM_EXIT:
        case IDCANCEL:
          if (CONNECTED( npTTYInfo ))
             CloseConnection( hWnd ) ;
          EndDialog(hWnd,1) ;
          break;
             
        case IDM_OPTIONS_SAVE:
          SaveMaxFreq(hWnd) ;
          break;
          
        case IDM_SCAN_START:
        case IDC_WIN_START:
        case IDM_SCAN_STOP:
        
            if ((wParam == IDM_SCAN_START) || 
             ((wParam == IDC_WIN_START) && (scanMode == FALSE))) {

            i = (myDialogBox( hWnd, "IDD_START_SCAN",(FARPROC) ScanStartProc));
            
            if (i == 0) { 
              MessageBox( hWnd, "Get Start Params Error.", " ",
                   MB_ICONEXCLAMATION ) ;
            }
            
            if (i == 2) {
              hMenu = GetMenu(hWnd);
            
              EnableMenuItem( hMenu, IDM_SCAN_START,
                   MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
              EnableMenuItem( hMenu, IDM_SCAN_STOP,
                   MF_ENABLED | MF_BYCOMMAND ) ;
              printW(hWnd,IDC_WIN_START, (LPSTR) " Stop ") ;

              scanMode = TRUE;
              scanState = 0;
              scanRepCount = 0;
              repaintFreqScale(hWnd,FALSE);
              freqReport(hWnd);
              
            }
      }
      else
      if (( wParam == IDM_SCAN_STOP) ||
             ((wParam == IDC_WIN_START) && (scanMode == TRUE))) {
             scanClose(hWnd);
         }
      if (wParam == IDM_OPTIONS_COMMAND) {
           i = myDialogBox( hWnd, "IDD_COMMAND",(FARPROC) CommandProc); 
           if (i==FALSE) 
             MessageBox( hWnd, "Get Command String Error.", " ",
                   MB_ICONEXCLAMATION ) ;
           if (i == TRUE) {
             if (!CONNECTED( npTTYInfo ))
               MessageBox( hWnd, "Connection failed.", " ",
                   MB_ICONEXCLAMATION ); 
             else {      
               WriteComm( COMDEV( npTTYInfo ), commStr, strlen(commStr) ) ;
	           b = ASCII_CR;
		       WriteCommByte( hWnd, b );
		     }
		   }
		 break;
		 }  
      }
      break;
         
    case WM_SIZE: 
         resizePicWin(hWnd,&scanWin);
         repaintFreqScale(hWnd,FALSE);
         //freqReport(hWnd);
         break;
              
    case WM_COMMNOTIFY:
         ProcessCOMMNotification( hWnd, (WORD) wParam, (LONG) lParam ) ;
         break ;
    
    case WM_PAINT:
           repaintFreqScale(hWnd,TRUE);
           //freqReport(hWnd);
         break;

    case WM_INITDIALOG:
         initSmallWindow(&mainWin,"Search");  
         
         CreateTTYInfo( hWnd ) ;
         if (!OpenConnection( hWnd ))
           MessageBox( hWnd, "Connection failed.", " ",
                   MB_ICONEXCLAMATION ) ;
         scS.hV = NULL;
         scS.pV = NULL;
                   
         initResizePicWin(hWnd,&scanWin);
         
         if (initFreqData(3000, 20000000, 1000) == FALSE)
           MessageBox( hWnd, "init Data failed.", " ",
                   MB_ICONEXCLAMATION ) ;
         repaintFreqScale(hWnd,FALSE);
         freqReport(hWnd);
         
         if (SetTimer(hWnd,779, 50,NULL) == 0) 
           MessageBox( hWnd, " Can't Set Timer. ", " ",
                   MB_ICONEXCLAMATION ) ;
            
         return TRUE;

	case WM_DESTROY:
         if (CONNECTED( npTTYInfo ))
		   CloseConnection( hWnd );
		 if (clearFreqPaint() == FALSE)   
           MessageBox( hWnd, "clear Data failed.", " ",
                   MB_ICONEXCLAMATION ) ;
         if (KillTimer(hWnd,779) == 0) 
           MessageBox( hWnd, " Can't Kill Timer. ", " ",
                   MB_ICONEXCLAMATION ) ;
         clearSmallWindow(&mainWin);          
        break;    
        
    case WM_TIMER:
       if ( (scanMode == TRUE) )
         scanSendStep(hWnd);
       if ( ( scanRepCount > 100 ) || ((scanRepCount != 0) && (scanMode == FALSE))) {
           repaintFreqScale(hWnd,FALSE);
           freqReport(hWnd);
           scanRepCount = 0;
       }
        break;    
    }
    return FALSE;
}

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
                if ( abIn[0] == '%' ) 
                  abIn[0] = 'A';
                //if (( abIn[0] < 'A' ) || ( abIn[0] > ('A'+15 ))) 
                //  abIn[0] = 'A';

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

int myDialogBox( HWND hWnd, LPCSTR lpszTemplate, FARPROC lpDlgProc)
// запуск диалога.
{
  extern HANDLE ghInst;   // app instance handle
  FARPROC     fpfn;
  int i;
    
    fpfn = MakeProcInstance(lpDlgProc, ghInst);
    i = DialogBox(ghInst, lpszTemplate, hWnd, (DLGPROC)fpfn);
    FreeProcInstance(fpfn);
        
  return i;
} 


// Resize Pic Win.

VOID resizePicWin ( HWND hWnd, PICTURE_TEXT_BUTTON_WINDOW *picWin ) {
  HWND hWndB;
  RECT lpcS,lpcW;
  int h,l,hm,lm;
                 
  GetClientRect(hWnd,&lpcW);
         
  lm = lpcW.right;

  hm = lpcW.bottom;
  
  if (hm < picWin->hPMin) hm = picWin->hPMin;
  
  if (lm < picWin->lPMin) lm = picWin->lPMin; 
         
  // Picture
  
  hWndB = GetDlgItem(hWnd,IDC_WIN_PICTURE);

  MoveWindow(hWndB,picWin->dLPicture,picWin->dHTop,lm-picWin->dLPicture*2,hm-picWin->dHBottom-picWin->dHTop,TRUE);

  // Button
  
  hWndB = GetDlgItem(hWnd,IDC_WIN_START);

  GetClientRect(hWndB,&lpcS);
  
  l  = lpcS.right;
  h  = lpcS.bottom;

  MoveWindow(hWndB,(lm-l) / 2, hm-picWin->dHButton, l,h,TRUE);

  // Text
  
  hWndB = GetDlgItem(hWnd,IDC_WIN_TEXT);

  GetClientRect(hWndB,&lpcS);
  
  l  = lpcS.right;
  h  = lpcS.bottom;

  MoveWindow(hWndB,picWin->dLPicture*2, hm-picWin->dHText, l,h,TRUE);
}

VOID initResizePicWin ( HWND hWnd, PICTURE_TEXT_BUTTON_WINDOW *picWin ) {
  HWND hWndB,hWndT;
  RECT lpcS,lpcW;
  int h,l,hm,lm;
  WORD wH;               

  GetWindowRect(hWnd,&lpcW);

  hWndB = GetDlgItem(hWnd,IDC_WIN_START);
  GetWindowRect(hWndB,&lpcS);
  
  picWin->dHButton = lpcW.bottom-lpcS.top;

  hWndT = GetDlgItem(hWnd,IDC_WIN_TEXT);
  GetWindowRect(hWndT,&lpcS);
  
  picWin->dHText = lpcW.bottom-lpcS.top;

  GetClientRect(hWndB,&lpcS);
  
  picWin->hPMin = lpcS.bottom;
  
  picWin->lPMin = lpcS.right;

  hWndB = GetDlgItem(hWnd,IDC_WIN_PICTURE);

  GetWindowRect(hWndB,&lpcS);

  picWin->dHBottom = lpcW.bottom-lpcS.bottom;
  
  GetClientRect(hWnd,&lpcW);
  GetClientRect(hWndB,&lpcS);

  l  = lpcS.right;
  h  = lpcS.bottom;
         
  lm = lpcW.right;
  hm = lpcW.bottom;
  
  picWin->dLPicture = (lm-l)/2;
  
  if ( picWin->dLPicture < 1 ) picWin->dLPicture = 1;
  
  picWin->dHTop = hm-h-picWin->dHBottom;
  
  if ( picWin->dHTop < 1 ) picWin->dHTop = 1;
  
  picWin->hPMin += (picWin->dHTop+picWin->dHBottom);
  picWin->lPMin += (picWin->dLPicture*2);
  
  GetWindowRect(GetDesktopWindow(), &lpcW);
  
  wH = (lpcW.bottom-lpcW.top)*3/5;
  
  MoveWindow(hWnd, 0, 0, lpcW.right-lpcW.left, wH, TRUE);
  
  resizePicWin ( hWnd, picWin );
}

