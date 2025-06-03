// Моно локатор:
//  (оконная функция).

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include 	"resource.h"
#include 	"sound.h"

HANDLE      ghInst      = NULL;   // instance handle

long	countW;  // Признак, что записываем.

VOID        RecPaint( HWND hWnd, BOOL pMode )      
{
}

BOOL FAR PASCAL __export RecordTest(HWND hWnd, unsigned wMsg, WORD wParam, LONG lParam)
{
    short i;
    FARPROC     fpfn;
    HMENU hMenu;

    switch (wMsg)
    {
    case WM_INITDIALOG:

        initSmallWindow(&lpcWin,"Compare");
        initSmallWindow(&waveWin,"Line In");
        initSmallWindow(&mainWin,"Locate");

        countW = -1;
        
        dBugL = 0; 

        InitData(hWnd);

        InitBuffers(hWnd);
        OpenBuffer(hWnd, 0,sizeof(ACUBUFF_IN),777);
        if (SetTimer(hWnd,779,200,NULL) == 0) 
          messageWin(hWnd, " Can't Set Timer. ");

        //if (SetTimer(hWnd,779,5000,NULL) == 0) 
        //  messageWin(hWnd, " Can't Set Timer. ");

        midiInit(hWnd,"shhh.mid","channel");
        //dSpeakers = 250;
        //printW(hWnd,IDC_SP_DIST,"%d",dSpeakers);
        return TRUE;

    case WM_PAINT:
        RecPaint(hWnd,TRUE);
        break;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDM_OPTIONS_MIXER:
            WinExec("C:\\jazz\\jazzpmix.exe",SW_SHOW /*MINIMIZED*/ );
            //WinExec("C:\\SB16\\winappl\\sb16wmix.exe",SW_SHOW);
            break;
            
        case IDM_WAVE:
    	    MakeWaveWin(hWnd,ghInst);
    	    break;

        case IDM_COMPARE:
    	    MakeLpcWin(hWnd,ghInst);
    	    break;
    	    
        case IDM_LOCATE_START:
        case IDM_LOCATE_STOP:
        case IDC_START:
            
            if ( (wParam == IDM_LOCATE_START) || 
                ((wParam == IDC_START) && ( countW <= 0 ))) {
			hMenu = GetMenu(hWnd);
           
            EnableMenuItem( hMenu, IDM_LOCATE_START,
                   MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
            EnableMenuItem( hMenu, IDM_LOCATE_STOP,
                   MF_ENABLED | MF_BYCOMMAND ) ;

            countW = 2;
            printW(hWnd,IDC_START,"&Stop");
            UpdateWindow (hWnd);
            break;
            }

            if ( (wParam == IDM_LOCATE_STOP) || 
                ((wParam == IDC_START) && ( countW > 0 )) ) {
            hMenu = GetMenu(hWnd);
            
            EnableMenuItem( hMenu, IDM_LOCATE_STOP,
                   MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
            EnableMenuItem( hMenu, IDM_LOCATE_START,
                   MF_ENABLED | MF_BYCOMMAND ) ;

            countW = 0;
            printW(hWnd,IDC_START,"&Start");
			
            UpdateWindow (hWnd);
            }
            break;
                  
        case IDC_CLEAR:     // "Clear"
        case IDM_LOC_CLEAR:
            for ( i = 0; i < NSAMPLE_IN; i++ ) {
              sr[i] = 0;
              sl[i] = 0;
            }
            
            for ( i = 0; i < MCORR; i++ ) {
              corrL[i] = 0;
            }
            
            clearBuffers(hWnd);
            RecPaint(hWnd,FALSE);
            printW(hWnd,IDC_DISTANCE,"");
            break;
            
        case IDM_HELP_ABOUT:
        case IDM_LOCATEHELP:
            break ;
            
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
            midiClose(hWnd,"channel");
	        CloseBuffers(hWnd);
            if (KillTimer(hWnd,779) == 0) 
              messageWin(hWnd," Can't kill timer. ");
            clearSmallWindow(&lpcWin);
            clearSmallWindow(&waveWin);  
            clearSmallWindow(&mainWin);  
        break;    
        
    case WM_TIMER:
        if ( wParam == 779 ) {
        if ( midiStatus(hWnd,"channel") == 3 ) {
          if ( countW == 2 ) {
            countW = 1;
            if ( RecordSoundOpen(hWnd, 0) == TRUE ) {
              midiPlay(hWnd,"channel");
              RecordSound(hWnd, 0);
            }  
          }  
        }
        }
        else {
          i = 0;
        }
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
        printW(hWnd,IDC_DISTANCE,"Distance   %7.2f m",(float)dBugL*(331.0/11025.0));
        
        if ( countW > 0 )
          countW = 2;
        else {
        }
        break;
    }      
    return FALSE;
}

