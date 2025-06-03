BUFFER_LENGTH 128
________________________________________

SendString -- процедура записи командной последовательности
в музыкольный синтезатор мультимедиа устройства и получения 
статуса синтезатора.
_________________________________________

initMidi - инийиализация МИДИ усторйства
  SendString
_________________________________________

midiPlay
  SendString
_________________________________________

midiClose
  SendString
_________________________________________

midiStatus 
_________________________________________

WinMain
    /* Display our dialog box. */
 myDialogBox( NULL, "IDD_START", (FARPROC) ChooseTest);

    for (;;) {
          
      switch (i) {        
        case 2 :
   		  i = myDialogBox( NULL, "IDD_FSEARCH", (FARPROC) ScanTest);
   		  break;
   		case 3 :
          i = myDialogBox( NULL, "IDD_CHECK", (FARPROC) CheckTest);
          break;
   		case 4 :
          i = myDialogBox( NULL, "IDD_LOCATOR", (FARPROC) RecordTest);
          break;
          
        case 1 :
          return TRUE;
          
        default : return TRUE;  
          
      }    
    }
}
_________________________________________________________

ChooseTest
(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {

    switch (msg) {
	  case WM_INITDIALOG:
	    return (TRUE);

	  case WM_COMMAND:
	    if ( wParam == IDCANCEL) {
	      EndDialog(hDlg, 1);
		  return (TRUE);
	    }
	    if (wParam == IDC_START_SEARCH) {
		  EndDialog(hDlg, 2);
		  return (TRUE);
	    }
	    if (wParam == IDC_START_CHECK) {
		  EndDialog(hDlg, 3);
		  return (TRUE);
	    }
	    if (wParam == IDC_START_LOCATE) {
		  EndDialog(hDlg, 4);
		  return (TRUE);
	    }
	    break;
    }
    return (FALSE);
}

____________________________________________________________

RecordTest
    switch (wMsg)
    {
    case WM_INITDIALOG:

        initSmallWindow(&lpcWin,"Compare");
        initSmallWindow(&waveWin,"Line In");
        initSmallWindow(&mainWin,"Locate");
        InitData(hWnd);
        InitBuffers(hWnd);
        OpenBuffer(hWnd, 0,sizeof(ACUBUFF_IN),777);
        if (SetTimer(hWnd,779,170,NULL) == 0) 
          messageWin(hWnd, " Can't Set Timer. ");
        initMidi();
        return TRUE;

    case WM_PAINT:
        RecPaint(hWnd,TRUE);
        break;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDM_OPTIONS_MIXER:
            WinExec("C:\\SB16\\winappl\\sb16wmix.exe",SW_SHOW);
            break;
            
        case IDM_WAVE:
    	    MakeWaveWin(hWnd,ghInst);
    	    break;

        case IDM_COMPARE:
    	    MakeLpcWin(hWnd,ghInst);
    	    break;
    	    
        case IDM_LOCATE_START:
			hMenu = GetMenu(hWnd);
           
            EnableMenuItem( hMenu, IDM_LOCATE_START,
                   MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
            EnableMenuItem( hMenu, IDM_LOCATE_STOP,
                   MF_ENABLED | MF_BYCOMMAND ) ;

            countW = 2;
            UpdateWindow (hWnd);
            break;

        case IDM_LOCATE_STOP:
            hMenu = GetMenu(hWnd);
            
            EnableMenuItem( hMenu, IDM_LOCATE_STOP,
                   MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
            EnableMenuItem( hMenu, IDM_LOCATE_START,
                   MF_ENABLED | MF_BYCOMMAND ) ;

            countW = 0;
			
            UpdateWindow (hWnd);
            break;
                  
        case IDC_CLEAR:     // "Clear"
        case IDM_LOC_CLEAR:
            for ( i = 0; i < NSAMPLE_IN; i++ ) {
              sr[i] = 0;
              sl[i] = 0;
            }
            
            for ( i = 0; i < MCORR; i++ ) {
              corrL[i] = 0;
              corrR[i] = 0;
            }
            
            clearBuffers(hWnd);
            RecPaint(hWnd,FALSE);
            break;
            
        case IDM_HELP_ABOUT:
        case IDM_LOCATEHELP:
            break ;

        case IDM_SAVE:
            //saveBuffers(hWnd);
            break;
            
        case IDM_DRIVERTEST:
            
            fpfn = MakeProcInstance((FARPROC) DeviceTest, ghInst);
            DialogBox(ghInst, "IDD_DEV_TEST", hWnd, (DLGPROC)fpfn);
            FreeProcInstance(fpfn);
            
            break;

        case IDM_GOTO_SEARCH:
            EndDialog( hWnd, 2 );
            break;

        case IDM_GOTO_CHECK:
            EndDialog( hWnd, 3 );
            break;

        case IDM_EXIT:      
        case IDCANCEL:      // "Exit"
            EndDialog( hWnd, 1 );
            break;
        }
        break;

	case WM_DESTROY:
            midiClose(hWnd);
	        CloseBuffers(hWnd);
            if (KillTimer(hWnd,779) == 0) 
              messageWin(hWnd," Can't kill timer. ");
            clearSmallWindow(&lpcWin);
            clearSmallWindow(&waveWin);  
            clearSmallWindow(&mainWin);  
        break;    
        
    case WM_TIMER:
        if ( midiStatus(hWnd) == TRUE ) ;
        break;

    case MM_WOM_DONE:       // End of Play Buffer.
        if ( ((LPWAVEHDR)lParam)->dwUser != 777 ) {
          messageWin(hWnd," Unexpected WOM_DONE. ");
          break;
        }

        checkOk(hWnd,waveOutReset(hWaveOut),IO_OUT);
        CloseDevice( hWnd, IO_OUT );
        break;
        
    case MM_WIM_DATA: 		// End Of Record.
        
        if ( ((LPWAVEHDR)lParam)->dwUser != 777 ) {
          messageWin(hWnd," Unexpected WIM_DATA. ");
          break;
        }
        
        checkOk(hWnd,waveInReset(hWaveIn),IO_IN);
        CopyData(0);
        unprepareIn(hWnd,0);
        CloseDevice( hWnd, IO_IN );

        DoLowPass(hWnd);
        if (waveWin.On == TRUE)
          SendMessage(waveWin.hWnd, WM_COMMAND,1,0);
        if (lpcWin.On == TRUE)
          SendMessage(lpcWin.hWnd, WM_COMMAND,2,1);
 
        RecPaint(hWnd,FALSE);
        dBugL = maxDD(corrL,MCORR);
        printW(hWnd,IDC_DISTANCE,"%d ",dBugL);
        
        if ( countW > 0 )
          countW = 2;
        else {
        }
        break;
    }      
    return FALSE;
}

