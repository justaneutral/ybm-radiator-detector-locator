#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include 	"resource.h"
#include 	"sound.h"

UINT d, mode, md1, md2;

// #define WAVE_FORMAT_1M08       0x00000001       /* 11.025 kHz, Mono,   8-bit  */
// #define WAVE_FORMAT_1S08       0x00000002       /* 11.025 kHz, Stereo, 8-bit  */
// #define WAVE_FORMAT_1M16       0x00000004       /* 11.025 kHz, Mono,   16-bit */
// #define WAVE_FORMAT_1S16       0x00000008       /* 11.025 kHz, Stereo, 16-bit */
// #define WAVE_FORMAT_2M08       0x00000010       /* 22.05  kHz, Mono,   8-bit  */
// #define WAVE_FORMAT_2S08       0x00000020       /* 22.05  kHz, Stereo, 8-bit  */
// #define WAVE_FORMAT_2M16       0x00000040       /* 22.05  kHz, Mono,   16-bit */
// #define WAVE_FORMAT_2S16       0x00000080       /* 22.05  kHz, Stereo, 16-bit */
// #define WAVE_FORMAT_4M08       0x00000100       /* 44.1   kHz, Mono,   8-bit  */
// #define WAVE_FORMAT_4S08       0x00000200       /* 44.1   kHz, Stereo, 8-bit  */
// #define WAVE_FORMAT_4M16       0x00000400       /* 44.1   kHz, Mono,   16-bit */
// #define WAVE_FORMAT_4S16       0x00000800       /* 44.1   kHz, Stereo, 16-bit */


void ShowInpDevice(HWND hDlg)
{
  WAVEINCAPS wc;

  if ((d < 0) || (d >= md1)) return;
  
  if (waveInGetDevCaps(d,(LPWAVEINCAPS)&wc, sizeof(WAVEINCAPS))) {
     MessageBox(hDlg, "Err in waveInGetDevCaps",
                   NULL, MB_OK | MB_ICONEXCLAMATION);
        return;
  }
  
  printW(hDlg,IDC_DEVICE_CUR,"  Input Device   %d  ",d+1);
  printW(hDlg,IDC_MAN_ID, "Manifact. Ident   %d",wc.wMid);
  printW(hDlg,IDC_PROD_ID,"Product Ident   %d",wc.wPid);
  printW(hDlg,IDC_VERSION,"Driver Version   %d.%d",(wc.vDriverVersion/256),(wc.vDriverVersion % 256));
  printW(hDlg,IDC_CHANNEL,"Channels   %d",wc.wChannels);
  printW(hDlg,IDC_FORMAT, "Formats   %ld",wc.dwFormats);
  printW(hDlg,IDC_NAME,   "Product name   %s",(LPARAM)(LPSTR)wc.szPname);
  printW(hDlg,IDC_SUPPORT,"");
}

void ShowOutDevice(HWND hDlg)
{                                
  WAVEOUTCAPS wc; 

  if ((d < 0) || (d >= md2)) return;
  
  if (waveOutGetDevCaps(d,(LPWAVEOUTCAPS)&wc, sizeof(WAVEINCAPS))) {
     MessageBox(hDlg, "Err in waveInGetDevCaps",
                   NULL, MB_OK | MB_ICONEXCLAMATION);
        return;
  }
           
  printW(hDlg,IDC_DEVICE_CUR,"  Output Device   %d  ",d+1);  
  printW(hDlg,IDC_MAN_ID, "Manifact. Ident   %d",wc.wMid);
  printW(hDlg,IDC_PROD_ID,"Product Ident   %d",wc.wPid);
  printW(hDlg,IDC_VERSION,"Driver Version   %d.%d",(wc.vDriverVersion/256),(wc.vDriverVersion % 256));
  printW(hDlg,IDC_CHANNEL,"Channels   %d",wc.wChannels);
  printW(hDlg,IDC_FORMAT, "Formats   %ld",wc.dwFormats);
  printW(hDlg,IDC_NAME,   "Product name   %s",(LPARAM)(LPSTR)wc.szPname);
  printW(hDlg,IDC_SUPPORT,"Support   %lu",wc.dwSupport);

/*  printW(hDlg,IDC_SUPPORT,"");
  printW(hDlg,IDC_DEVICE_CUR," Output Device Number %d ",d+1);  
  printW(hDlg,IDC_MAN_ID,"Manifact. Id %d",wc.wMid);
  printW(hDlg,IDC_PROD_ID,"Product Id %d",wc.wPid);
  printW(hDlg,IDC_VERSION,"Driver Version %d.%d",(wc.vDriverVersion/256),(wc.vDriverVersion % 256));
  printW(hDlg,IDC_CHANNEL,"Channels %d",wc.wChannels);
  printW(hDlg,IDC_FORMAT,"Formats %ld",wc.dwFormats);
  printW(hDlg,IDC_NAME,"Product name %s",(LPARAM)(LPSTR)wc.szPname);
  */
}

VOID showNextDevice(HWND hDlg) {

  if ( mode == TRUE ) {
    if ( d < md1 )
      ShowInpDevice(hDlg);
  }
  else {
    if ( d < md2 )
      ShowOutDevice(hDlg);
  }

  d++;
  if ( mode == TRUE ) {
    if ( d >= md1 ) {
      mode = FALSE;
      d = 0;
    }  
  }
  else {
    if ( d >= md2 ) {
      mode = TRUE;
      d = 0;
    }  
  }
}

BOOL FAR PASCAL __export DeviceTest(HWND hDlg, unsigned msg, WORD wParam, LONG lParam)
{   
    switch (msg)
    {
    case WM_COMMAND:
        if (wParam == IDOK)
            EndDialog(hDlg,TRUE);
        if (wParam == IDC_NEXT) 
          showNextDevice(hDlg);
        break;

    case WM_INITDIALOG:
        mode = TRUE;
        d = 0;
        
        md1 = waveInGetNumDevs();
        printW(hDlg,IDC_NUM_INP,"Input %d",md1);
        md2 = waveOutGetNumDevs();
        printW(hDlg,IDC_NUM_OUT,"Output %d",md2);
        
        if (md1 < 1) {
          mode = FALSE;
          if (md2 < 2)
            EnableWindow(GetDlgItem(hDlg,IDC_NEXT), FALSE);
        }  

        showNextDevice(hDlg);
          
        return TRUE;
    }
    return FALSE;
}
  
