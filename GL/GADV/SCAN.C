#include "cinc.h"
#include "fcomm.h"
#include <stdlib.h>
#include "scan.h"

static BOOL  ScanMode = FALSE;

static int scanRepCount = 0;

static long LastRepMax = 0;

VOID freqReport(HWND hWnd) {      // Text String
  long tf;
  
  tf = tV.f;
  if ( tf < CurTab.FirstFreq )
    tf = CurTab.FirstFreq;
  if ( tf > CurTab.LastFreq )
    tf = CurTab.LastFreq;  
    
  printW(hWnd,IDC_SCAN_STATE,
  "From  %ld.%04ld mHz  (%ld.%04ld mHz)  to  %ld.%04ld mHz.",
    (CurTab.FirstFreq/10000),(CurTab.FirstFreq%10000),
    (tf/10000),    (tf%10000),
    (CurTab.LastFreq/10000),(CurTab.LastFreq%10000) );
}

static VOID scanClose(HWND hWnd) {       // Stop Scan.

  ScanMode = FALSE;

  reDrawScale();

  freqReport(hWnd);

  scanRepCount = 0;
}


BOOL FAR PASCAL __export AR3000DialogProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {
// Test Mode (Send String) Dialog.
    switch (msg) {
	  case WM_INITDIALOG:
        clearCommStr();
	    return (TRUE);

	  case WM_COMMAND:
	    if ( wParam == IDCANCEL) {
	        EndDialog(hDlg, 2);
		  return (TRUE);
	    }
	    if (wParam == IDOK) {
          if (!GetWindowText( GetDlgItem(hDlg,IDC_COMMAND), (LPSTR)commStr, 15)) {
            clearCommStr();
            EndDialog(hDlg, FALSE);
          }  
	 	  else
	        EndDialog(hDlg, TRUE);
 		  return (TRUE);
	    }
	    break;
    }
    return (FALSE);
}

long doubleToLong( double f ) {
  return (long)f;
}  

BOOL FAR PASCAL __export StartScanDialogProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {
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
              Message( hDlg, s ) ;
              return (FALSE);
            }

            nf1 = doubleToLong(f*10000.0);

            if (!GetWindowText( GetDlgItem(hDlg,IDC_GET_F2), (LPSTR)commStr, 15)) {
              EndDialog(hDlg, 0);
              break;
            }  

            f = strtod(commStr,&s);
            
            if ( s[0] != NULL ) {
              Message( hDlg, s ) ;
              return (FALSE);
            }

            nf2 = doubleToLong(f*10000.0);

            if (!GetWindowText( GetDlgItem(hDlg,IDC_GET_FS), (LPSTR)commStr, 15)) {
              EndDialog(hDlg, 0);
              break;
            }
            
            f = strtod(commStr,&s);
            
            if ( s[0] != NULL ) {
              Message( hDlg, s ) ;
              return (FALSE);
            }
          
            nfs = doubleToLong(f*10.0);
            
            if ( nf1 > nf2 ) {
              Message( hDlg, " f1 > f2 " ) ;
              return (FALSE);
            }
          
            if ( nfs < 1 ) {
              Message( hDlg, " Error frequency step " ) ;
              return (FALSE);
            }
            // инит здесь:
            
            CurTab.FirstFreq = nf1;
            CurTab.LastFreq = nf2;
            EndDialog(hDlg, 2);
            return (TRUE);
            break;
        }
        break;
    }
    return (FALSE);
}

static VOID ScanOn(HWND hWnd, BOOL St) {

  if ( CommonError ) 
    return;  

  ColorsOn(St);
  
  FreqScaleOn(&CurTab,St);
  
  ConnectionOn( hWnd, St );

  if ( St ) {
    ClearMaxScan( &CurTab );
    InitScaleWin("Frequency Scale");
    freqReport(hWnd);
  }
}


BOOL FAR PASCAL __export ScanDialogProc(HWND hWnd, unsigned msg, WORD wParam, LONG lParam)
{   
  //BYTE b;
  int i;
  
  switch (msg) {
    case WM_COMMAND:
      switch (wParam) {

        case IDCANCEL:
          EndDialog(hWnd,1) ;
          break;

        case IDC_MODE:
          EndDialog(hWnd,2) ;
          break;

        case IDC_STOP :
          scanClose(hWnd);
          break;
           
        case IDC_WINDOW :
          MakeScaleWin(hWnd);
          break;
             
        case IDC_SAVE :
          SaveFreqScale(hWnd,"freqmax.dat",&CurTab);
          break;

        case IDC_COMMAND :
          if ( ScanMode or CommonError )
            break;
             
          i = myDialogBox( hWnd, "IDD_COMMAND",(FARPROC) AR3000DialogProc); 

          if (i == TRUE) {
            if (!npTTYInfo.fConnected)
              Message( hWnd, "Connection failed." ); 
            else {
              strcat(commStr,"\r");
              WriteComm( npTTYInfo.idComDev, commStr, strlen(commStr) ) ;
              clearCommStr();
		    }
		  }
		  break;

        case IDC_START:
          if ( ScanMode or CommonError )
            break;

          i = (myDialogBox( hWnd, "IDD_START_SCAN",(FARPROC) StartScanDialogProc));
            
          if (i == 0)  
            Message( hWnd, "Get Start Params Error." ) ;
            
          if (i == 2) {
            ClearMaxScan(&CurTab);
            scanRepCount = 0;
            clearCommStr();
            reDrawScale();
            freqReport(hWnd);                        
            ScanMode = TRUE;
            CommSendStr( hWnd );
          }
          break;
      }
      break;

    case WM_COMMNOTIFY:
         if ( ProcessCOMMNotification( hWnd, (WORD) wParam, (LONG) lParam ) == TRUE ) {
           if ( ScanMode )
             CommSendStr( hWnd );
         }
         return TRUE;

    case WM_INITDIALOG:
         ScanOn(hWnd,TRUE);
         
         //MakeScaleWin(hWnd);
   
         return TRUE;

	case WM_DESTROY:
	     ScanOn(hWnd,FALSE);
         break;    
        
    }
    return FALSE;
}

