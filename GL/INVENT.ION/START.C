
//  Начало работы:
//     выбор компоненты.

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include 	"resource.h"
#include 	"sound.h"


int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int iCmdShow)
{
    // FARPROC     fpfn;
    int i;
    
    /* Save instance handle for dialog boxes. */
    
    ghInst = hInst;
    mainWin.Cl = FALSE;
    waveWin.Cl = FALSE;
    lpcWin.Cl = FALSE;

    /* Display our dialog box. */
    
    i = myDialogBox( NULL, "IDD_START", (FARPROC) ChooseTest);

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

BOOL FAR PASCAL __export ChooseTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {

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

