#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include "fcomm.h"
#include "scnpaint.h"
#include "resource.h"
#include "sound.h"
#include "fft.h"

static BOOL  checkMode = FALSE,
             midiChStatus = FALSE; // check or idle.
      
static UINT selectedMax = 0;
static long selectedPrv = -1;
static HWND hWndCheck = NULL;
static long nOfChecks = 0;

static enum {	check_Idle,
      	check_SetFreq,
      	check_Hear,
      	check_Hear_WaitM
     } checkState;
     
static int  checkRepCount = 0; // for graph output.

static UINT checkPos = 0; // position in the buffer.

static PICTURE_TEXT_BUTTON_WINDOW checkWin;

BOOL FAR ProcessCheckCOMMNotification( HWND hWnd, WORD wParam, LONG lParam );

VOID    repaintCheckScale(HWND hWnd, BOOL mode) {
  double prm;
  UINT i;
  int vch;
  
  prm = 1;
  
  for (i = 0; i < scS.nMax; i++)
    if (afm[i].pr > prm) 
      prm = afm[i].pr;
      
  for (i = 0; i < scS.nMax; i++) {
    vch = round(afm[i].pr*16.0/prm);
    if (vch < 1) vch = 1;
    if (vch > 15) vch = 15;
    scS.pV[afm[i].ac] = vch;
  }
    
  repaintFreqScale(hWnd,mode);
}


VOID checkClose(HWND hWnd) {
  HMENU hMenu;
  
  hMenu = GetMenu(hWnd);
  EnableMenuItem( hMenu, IDM_CHECK_STOP,
              MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
  EnableMenuItem( hMenu, IDM_CHECK_START,
              MF_ENABLED | MF_BYCOMMAND ) ;
  printW(hWnd,IDC_WIN_START, (LPSTR) "Start") ;
  checkMode = FALSE;
}

VOID freqChReport(HWND hWnd) {
  long tf;
  
  tf = afm[checkPos].f;  
  
  printW(hWnd,IDC_WIN_TEXT,"From %ld.%04ld mHz  (%ld.%04ld mHz)  to %ld.%04ld mHz.  Check %ld.",
    (scS.f1/10000),(scS.f1%10000),
    (tf/10000),(tf%10000),
    (scS.f2/10000),(scS.f2%10000),
    nOfChecks);
    //sl[absD(sl,NSAMPLE_CH)],sr[absD(sr,NSAMPLE_CH)]);
}

int HearMax( HWND hWnd) {
  if ( RecordSoundOpen(hWnd, 0) == TRUE ) {
    RecordSound(hWnd, 0);
  }  

  return 0;
}

int checkSendStep(HWND hWnd) {
  long tf;
  
  if (checkState == check_SetFreq) {
    checkPos++;
    if (checkPos >= scS.nMax) {
      nOfChecks++;
      checkPos = 0;
    }  
      
    tf = afm[checkPos].f;  
  
    wsprintf(commStr,"%ld.%04ld\r",(tf/10000),(tf%10000));
    WriteComm( COMDEV( npTTYInfo ), commStr, strlen(commStr) ) ;
  
    checkState = check_Hear;
    return 0;  
  }
  if (checkState == check_Hear) {
    HearMax(hWnd);
    checkState = check_Hear_WaitM;
    return 0;
  }
  return 0;
}


int setFreq (HWND hWnd) {
  long tf;
  
  if (scS.nMax < 1)
    return FALSE;
       
  if (selectedMax >= scS.nMax) 
    selectedMax = 0;
      
  tf = afm[selectedMax].f;  
  
  wsprintf(commStr,"%ld.%04ld\r",(tf/10000),(tf%10000));
  WriteComm( COMDEV( npTTYInfo ), commStr, strlen(commStr) ) ;
    
  printW(hWnd,IDC_SELECT_F,"F %ld.%04ld mHz",(tf/10000),(tf%10000));

  if ( selectedPrv >= 0 )
    marker(FALSE, hWndCheck, afm[selectedPrv].ac, scS.nV, IDC_WIN_PICTURE);
  marker(FALSE, hWndCheck, afm[selectedMax].ac, scS.nV, IDC_WIN_PICTURE);
  selectedPrv = selectedMax;
  
  return TRUE;
}


BOOL FAR PASCAL __export SelectFreq(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {

    switch (msg) {
	  case WM_INITDIALOG:
	    setFreq(hDlg);
	    return (TRUE);
	    
	  case WM_DESTROY:
	    if ( selectedPrv >= 0 )
          marker(FALSE, hWndCheck, afm[selectedPrv].ac, scS.nV, IDC_WIN_PICTURE);
        break;  

	  case WM_COMMAND:
	    if ( wParam == IDCANCEL) {
	        EndDialog(hDlg, 2);
		  return (TRUE);
	    }
	    if (wParam == IDOK) {
	        EndDialog(hDlg, TRUE);
		  return (TRUE);
	    }
	    if (wParam == IDC_BEST) {
	      selectedMax = 0;
	      setFreq(hDlg);
	    }
	    if (wParam == IDC_NEXT) {
	      selectedMax++;
	      setFreq(hDlg);
	    }
	    if (wParam == IDC_PREV) {
	      if (selectedMax >= 1)
	        selectedMax--;
	      setFreq(hDlg);  
	    }
	    break;
    }
    return (FALSE);
}

BOOL FAR PASCAL __export CheckTest(HWND hWnd, unsigned msg, WORD wParam, LONG lParam)
{   
  HMENU hMenu;
  
  switch (msg) {
    case WM_COMMAND:
      switch (wParam) {
      
        case IDM_GOTO_SEARCH:
            if (CONNECTED( npTTYInfo ))
              CloseConnection( hWnd ) ;
            EndDialog( hWnd, 2 );
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
        
        case IDM_OPTIONS_MIXER:
            //WinExec("C:\\SB16\\winappl\\sb16wmix.exe",SW_SHOW /*MINIMIZED*/ );
            WinExec("C:\\jazz\\jazzpmix.exe",SW_SHOW /*MINIMIZED*/ );
            break;
            
        case IDM_WAVE:
    	    MakeWaveWin(hWnd,ghInst);
    	    break;
    	    
    	case IDM_COMPARE:
    	    MakeLpcWin(hWnd,ghInst);
    	    break;

    	case IDM_OPTIONS_SAVE:
    	    SaveMaxFreqAFM(hWnd);
    	    break;
    	    
    	case IDM_CLEAR:
    	    {
    	      UINT i;
              
              for ( i = 0; i < scS.nMax; i++ ) {
                //scS.pV[curMax.ac] = curMax.v;
                //afm[i].v = curMax.v;
                //afm[i].ac = curMax.ac;
                afm[i].pr = 0.0;
                orderAFM();
                //afm[i].f = curMax.f;
              }  
            }
	        repaintCheckScale(hWnd,FALSE);
    	    break;    
    	    
    	case IDM_SELECT:
            {
    	    UINT i;
    	    
    	    if ( checkMode == FALSE ) {
    	      selectedMax = 0;
    	      selectedPrv = -1;
    	      sortAFM();
    	      hWndCheck = hWnd;
              i = myDialogBox( hWnd, "IDD_SELECT",(FARPROC) SelectFreq); 
              if (i==FALSE) MessageBox( hWnd, "?", " ",MB_ICONEXCLAMATION ) ;
            }
    	    }
    	    break;  
    	    
    	case IDM_CHECK_STOP:
    	case IDM_CHECK_START:
    	case IDC_WIN_START:
    	    
          if (( wParam == IDM_CHECK_STOP) ||
             ((wParam == IDC_WIN_START) && (checkMode == TRUE))) {
             checkClose(hWnd);
             midiClose(hWnd,"channel");
          }
          else
          if ((wParam == IDM_CHECK_START) ||
             ((wParam == IDC_WIN_START) && (checkMode == FALSE))) {
            if ( (scS.nMax < 1) || (afm == NULL) ) {
              MessageBox( hWnd, "Empty freq scale.", " ",
                   MB_ICONEXCLAMATION ) ;
          }
          else {           
            hMenu = GetMenu(hWnd);
            
            EnableMenuItem( hMenu, IDM_CHECK_START,
              MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
            EnableMenuItem( hMenu, IDM_CHECK_STOP,
              MF_ENABLED | MF_BYCOMMAND );
            printW(hWnd,IDC_WIN_START, (LPSTR) " Stop ") ;
   
            checkMode = TRUE;
            checkState = check_SetFreq;
            checkPos = scS.nMax;
            checkRepCount = 0;
			nOfChecks = 0;
            midiChStatus = FALSE;
            orderAFM();
	        repaintCheckScale(hWnd,FALSE);
            midiInit(hWnd,"hellomyb.mid","channel");
            midiPlay(hWnd,"channel");          
          }
        }
      }
      break;
         
    case WM_SIZE: 
         resizePicWin(hWnd,&checkWin);
         repaintFreqScale(hWnd,FALSE);
         break;
              
    case WM_COMMNOTIFY:
         ProcessCheckCOMMNotification( hWnd, (WORD) wParam, (LONG) lParam ) ;
         break ;
    
    case WM_PAINT:
           repaintFreqScale(hWnd,TRUE);
         break;

    case WM_INITDIALOG:
         initFft();
         CreateTTYInfo( hWnd ) ;
         if (!OpenConnection( hWnd ))
           MessageBox( hWnd, "Connection failed.", " ",
                   MB_ICONEXCLAMATION ) ;
         scS.hV = NULL;
         scS.pV = NULL;

		 hAFM 	= NULL;     // array of freq max's.
         afm 	= NULL;		// -""-

         checkState = check_Idle;
         checkMode = 0;
                   
         initResizePicWin(hWnd,&checkWin);
         
         if (LoadMaxFreq(hWnd) == FALSE)
           MessageBox( hWnd, "Load Freq Max failed.", " ",
                   MB_ICONEXCLAMATION ) ;
         repaintCheckScale(hWnd,FALSE);
         freqChReport(hWnd);
         if (SetTimer(hWnd,779, 50,NULL) == 0) 
           MessageBox( hWnd, " Can't Set Timer. ", " ",
                   MB_ICONEXCLAMATION ) ;

         InitBuffers(hWnd);
         OpenBuffer(hWnd, 0, sizeof(ACUBUFF_IN), 777);

         //   InitBuffers(hWnd);
         //   OpenBuffer(hWnd, 0,sizeof(ACUBUFF_IN),777);
         initSmallWindow(&waveWin,"Line In");
         initSmallWindow(&lpcWin ,"Compare");
         initSmallWindow(&mainWin ,"Check");
         
         return TRUE;
         
    case MM_WIM_DATA: 		// End Of Record.        
         if ( ((LPWAVEHDR)lParam)->dwUser != 777 ) {
           messageWin(hWnd," Unexpected WIM_DATA. ");
           break;
         }

         if ( checkState != check_Hear_WaitM ) {
           messageWin(hWnd," Unexpected Check State. ");
           break;
         }

         if ( afm == NULL ) {
           messageWin(hWnd," Unexpected AFM. ");
           break;
         }
        
         checkOk(hWnd, waveInReset(hWaveIn), IO_IN);

         afm[checkPos].pr += CopyDataCh(0);

         unprepareIn(hWnd,0);

         CloseDevice( hWnd, IO_IN );
         checkState = check_SetFreq;
         if (waveWin.On == TRUE)
           SendMessage(waveWin.hWnd, WM_COMMAND,1,0);
         if (lpcWin.On == TRUE)
           SendMessage(lpcWin.hWnd, WM_COMMAND,2,0);
         break;
         
	case WM_DESTROY:
	
         if (CONNECTED( npTTYInfo ))
		   CloseConnection( hWnd );
		 if (clearFreqPaint() == FALSE)   
           MessageBox( hWnd, "clear Data failed.", " ",
                   MB_ICONEXCLAMATION ) ;
                   
		 if (ClearAFMPtr(hWnd) == FALSE)   
           MessageBox( hWnd, "clear AFM failed.", " ",
                   MB_ICONEXCLAMATION ) ;
                   
         if (KillTimer(hWnd,779) == 0) 
           MessageBox( hWnd, " Can't Kill Timer. ", " ",
               MB_ICONEXCLAMATION ) ;
         CloseBuffers(hWnd);
   
         clearSmallWindow(&waveWin);
         clearSmallWindow(&lpcWin);
         clearSmallWindow(&mainWin);  
          
         break;    
        
    case WM_TIMER:
       if ( checkMode == TRUE ) {
         midiChStatus = midiStatus(hWnd,"channel");
         if (midiChStatus == 1) {
           checkSendStep(hWnd);
           if ( ( checkRepCount++ > 40 ) || 
              ((checkRepCount != 0) &&
              (checkMode == FALSE))) {
             repaintCheckScale(hWnd,FALSE);
             freqChReport(hWnd);
             checkRepCount = 0;
           }  
         }
         if (midiChStatus == 3) {
            midiPlay(hWnd,"channel");          
         }
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


BOOL FAR ProcessCheckCOMMNotification( HWND hWnd, WORD wParam, LONG lParam )
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
         if ( ( nLength = ReadCommBlock( hWnd, (LPSTR) abIn, MAXBLOCK )) != 0 ) {
         // ... get string.
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
      while ((!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE )) ||
            (ComStat.cbInQue >= MAXBLOCK)) ;
   }

   return ( TRUE ) ;

} // end of ProcessCOMMNotification()

