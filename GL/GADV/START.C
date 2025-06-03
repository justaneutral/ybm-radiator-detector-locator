
//  Начало работы: выбор компоненты.

#include "cinc.h"

HINSTANCE ghInst;

BOOL FAR PASCAL __export GantDialogProc(HWND hWnd, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export StartDialogProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export ScanDialogProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam);
BOOL FAR PASCAL __export CheckDialogProc(HWND hWnd, unsigned msg, WORD wParam, LONG lParam);

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int iCmdShow)
{
    // FARPROC     fpfn;
    int i;
    
    /* Save instance handle for dialog boxes. */
    
    ghInst = hInst;
    
    i = 3;
    
    for (;;) {
          
      switch (i) {        
        case 2 :
   		  i = myDialogBox( NULL, "IDD_SCAN", (FARPROC) ScanDialogProc);
   		  break;

   		case 3 :
          i = myDialogBox( NULL, "IDD_GANT", (FARPROC) GantDialogProc);
          break;
          
   		case 4 :
          i = myDialogBox( NULL, "IDD_CHECK", (FARPROC) CheckDialogProc);
          break;

        case 1 :
          return TRUE;
          
        default : return TRUE;  
          
      }
      if (i == 1) return TRUE;
      i = myDialogBox( NULL, "IDD_START", (FARPROC) StartDialogProc);
    }
}


BOOL FAR PASCAL __export StartDialogProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {

    switch (msg) {
	  case WM_INITDIALOG:
	    return (TRUE);

	  case WM_COMMAND:
	    if ( wParam == IDCANCEL) {
	      EndDialog(hDlg, 1);
		  return (TRUE);
	    }
	    if (wParam == IDC_SCAN_START) {
		  EndDialog(hDlg, 2);
		  return (TRUE);
	    }
	    if (wParam == IDC_GANT_START) {
		  EndDialog(hDlg, 3);
		  return (TRUE);
	    }
	    if (wParam == IDC_CHECK_START) {
		  EndDialog(hDlg, 4);
		  return (TRUE);
	    }
	    break;
    }
    return (FALSE);
}

