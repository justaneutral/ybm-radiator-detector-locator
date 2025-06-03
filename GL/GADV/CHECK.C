#include "cinc.h"
#include "fcomm.h"
#include <stdlib.h>
#include "scan.h"
#include "math.h"

      
static long selectedMax = 0;

static HWND hWndCheck;

//static BOOL CheckError = FALSE;
static BOOL CheckMode  = FALSE;
static BOOL WaveMode   = FALSE;

static long nOfChecks = 0;

static int  checkRepCount = 0; // for graph output.

static long checkPos = 0; // position in the buffer.

//static checkCommState = 0; // for ProcessCheckCOMMNotification;

#define NSUBBUF	   20
#define NSAMPLE_CH mFft
#define NSAMPLE_IN (mFft*NSUBBUF)
       
static FUNWIN fwFftX,fwFftY;  // дополнительные окошки для спектров
static DOUBLEARRAY slm,srm;

static WaveBuffer pb;


//
// Мат процедуры
//

static double Compare( void ) {
  UINT i,i0;
  double s,sn;
  
  //CopyWave(&pb);
  
  s = 0.0;
  sn = 0.0;
  
  
  for (i0 = 0; (i0+NSAMPLE_CH) <= NSAMPLE_IN; i0 += NSAMPLE_CH) {
  
    for ( i = 0; i < NSAMPLE_CH; i++ ) {
      slm[i] = sl[i+i0];
      srm[i] = sr[i+i0];
    }
  
    trend(srm);
    trend(slm);
  
    arrMul(fftX,srm,fftXemm);
    arrMul(fftY,slm,fftXemm);

    fft();
  
    fftAmp();
  
    logAmp();
  
    for ( i = 1; i < mFftH/2; i++ ) 
      s += (fftY[i]-fftY[i-1])*(fftX[i]-fftX[i-1]);

    for ( i = 1; i < mFftH/2; i++ ) 
      sn += (fftY[i]-fftY[i-1])*(fftY[i]-fftY[i-1])+
          (fftX[i]-fftX[i-1])*(fftX[i]-fftX[i-1]);
  }
  if (sn < 1.0) sn = 1.0;

  if ( fwFftX.On )  
    SendMessage( fwFftX.hWnd, WM_COMMAND, 100,0 );
  
  //if ( fwFftY.On )  
  //  SendMessage( fwFftY.hWnd, WM_COMMAND, 100,0 );

  return s/sqrt(sn);
}


//_________________________________________________
//

static VOID CheckReport( long f ) {
  long tf;
  
  if ( ( f < 0 ) or ( f >= CurTab.nMax ) )
    tf = 0;
  else  
    tf = CurTab.FreqMax[f].fMid.f;  
  
  if ( tf < CurTab.FirstFreq )
    tf = CurTab.FirstFreq;
  if ( tf > CurTab.LastFreq )
    tf = CurTab.LastFreq;  
    
  printW(hWndCheck,IDC_OUT,
  "From  %ld.%04ld mHz  (%ld.%04ld mHz)  to  %ld.%04ld mHz.  Check %ld.",
    (CurTab.FirstFreq/10000),(CurTab.FirstFreq%10000),
    (tf/10000),    (tf%10000),
    (CurTab.LastFreq/10000),(CurTab.LastFreq%10000),
    nOfChecks);
}


static VOID HearMax( HWND hWnd) {
  if ( CommonError or WaveMode ) 
    return;
  
  WaveMode = TRUE;
  WaveInp(hWnd,&pb);
}


static int setNextFreq(HWND hWnd) {
  long tf;

  if ( CommonError )
    return FALSE;
  if ( (CurTab.nMax < 1) or (CurTab.FreqMax == NULL) )
    return FALSE;
  
  checkPos++;
    
  if (checkPos >= CurTab.nMax) {
    nOfChecks++;
    checkPos = 0;
  }  
      
  tf = CurTab.FreqMax[checkPos].fMid.f;  
    
  clearCommStr();
  wsprintf(commStr,"%ld.%04ld\r",(tf/10000),(tf%10000));
  WriteComm( npTTYInfo.idComDev, commStr, strlen(commStr) ) ;
  
  return 0;
}


static long selectedPrv = -1;

static int setFreq (HWND hWnd) {
  long tf;
  
  if ( CommonError )
    return FALSE;
  if ( (CurTab.nMax < 1) or (CurTab.FreqMax == NULL) )
    return FALSE;
       
  if (selectedMax >= CurTab.nMax) 
    selectedMax = 0;
      
  tf = CurTab.FreqMax[selectedMax].fMid.f;  
  
  wsprintf(commStr,"%ld.%04ld\r",(tf/10000),(tf%10000));
  WriteComm( npTTYInfo.idComDev, commStr, strlen(commStr) ) ;
    
  //printW(hWnd,IDC_OUT,"F %ld.%04ld mHz",(tf/10000),(tf%10000));
  CheckReport(selectedMax);

  if ( selectedMax >= 0 )
    PutPowerMarker(selectedMax);

  selectedPrv = selectedMax;
  
  return TRUE;
}


BOOL FAR PASCAL __export SelectFreqDialogProc(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) {

    switch (msg) {
	  case WM_INITDIALOG:
	    setFreq(hDlg);
	    return (TRUE);
	    
	  case WM_DESTROY:
	    //if ( selectedPrv >= 0 )
        //  marker(FALSE, hWndCheck, afm[selectedPrv].ac, scS.nV, IDC_WIN_PICTURE);
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

static VOID CheckOn( HWND hWnd, BOOL St ) {

  if ( CommonError ) 
    return;

  hWndCheck = hWnd;
  
  DoubleOn(hWnd, NSAMPLE_CH, &slm, St);
  DoubleOn(hWnd, NSAMPLE_CH, &srm, St);
  
  DoubleOn(hWnd, NSAMPLE_IN, &sl, St);
  DoubleOn(hWnd, NSAMPLE_IN, &sr, St);
  midiOn(hWnd, "cannon_d.mid", "ch", St);
  
  if ( CommonError ) {
    return;
  }  

  ColorsOn(St);
  
  WaveOn(hWnd,St,1);

  FreqScaleOn(&CurTab,St);
  
  ConnectionOn( hWnd, St );
  
  FftOn ( hWnd, St );

  if ( St ) {
    InitWaveWindow();
    //OutPort(0);
    OpenBuffer(hWnd,NSAMPLE_IN*2*2,77,&pb);
    InitGraWin(&fwFftX,fftX,"FFT");
    InitGraWin(&fwFftY,fftY,"FFT Y");

    //ClearMaxScan( );
    //LoadFreqScale(hWnd);
    InitPowerScaleWin("Frequency Scale");
    CheckReport(0);
  }

  clearCommStr();
  CheckMode = FALSE;
}


static VOID ShowWin( HWND hWnd ) {
  MakeWaveWin(hWnd);
  MakeGraWin(hWnd,&fwFftX,1);
  //MakeGraWin(hWnd,&fwFftY);
  MakePowerScaleWin(hWnd);
  //MakeDirWin(hWnd,&fwDir);
}


BOOL FAR PASCAL __export CheckDialogProc(HWND hWnd, unsigned msg, WORD wParam, LONG lParam)
{   
  switch (msg) {
    case WM_COMMAND:
      switch (wParam) {

        case IDCANCEL:
            EndDialog(hWnd,1);
            break;

        case IDC_MODE:
            EndDialog(hWnd,2) ;
            break;

    	case IDC_SAVE:
    	    SaveFreqScale(hWnd,"check.dat",&CurTab);
    	    break;
    	    
    	case IDC_CLEAR:
	        reDrawPowerScale();
    	    break;    
    	    
    	case IDC_WINDOW:
	        ShowWin( hWnd );
    	    break;    

    	case IDC_CHOOSE:
            {
    	    UINT i;
              if ( (CurTab.nMax < 1) || (CurTab.FreqMax == NULL) ) {
                Message( hWnd, "Empty freq scale.");
                break;
              }  
              if ( CommonError ) 
                break;
    	    
     	      if ( not CheckMode ) {
    	        selectedMax = 0;
    	        selectedPrv = -1;
    	        sortFreqMax(&CurTab);
    	        //hWndCheck = hWnd;
                i = myDialogBox( hWnd, "IDD_SELECT",(FARPROC) SelectFreqDialogProc); 
                if (i == FALSE) Message( hWnd, "?" ) ;
                PutPowerMarker(-1);
              }
    	    }
    	    break;  
    	    
    	case IDC_STOP:
            CheckMode = FALSE;
            midiStop(hWnd,"ch");
            break;   

    	case IDC_START: ;
            if ( (CurTab.nMax < 1) || (CurTab.FreqMax == NULL) ) 
              Message( hWnd, "Empty freq scale.");
            else {   
              //checkState = check_Hear;
              checkPos = CurTab.nMax;
              checkRepCount = 0;
		      nOfChecks = 0;
              //sortFreqMax();
	          reDrawPowerScale();
              midiRePlay(hWnd,"ch");
            
              clearCommStr();
              CheckMode = TRUE;
              setNextFreq(hWnd);
            }
        }
        break;
              
    case WM_COMMNOTIFY:
         if (ProcessCOMMNotification(hWnd,(WORD)wParam,(LONG)lParam) == TRUE) {
           clearCommStr();
           if ( CheckMode )
             HearMax(hWnd);
         }  
         break ;

    case MM_WIM_DATA: 		// End Of Record.        
         if ( ((LPWAVEHDR)lParam)->dwUser != 77 ) {
           Message(hWnd," Unexpected WIM_DATA. ");
           break;
         }

         unprepareInp(hWnd,&pb);
         CloseDevice(hWnd,TRUE);
         WaveMode = FALSE;
         CopyWave(&pb);
         if ( waveWin.On )
           SendMessage(waveWin.hWnd, WM_COMMAND,100,0);

         CurTab.FreqMax[checkPos].power += Compare();

         {
           HDC rDC;
           
           rDC = GetDC(fwPowerScale.hWnd);
           if ( drawPowerMax(rDC, checkPos, checkPos+1) )
             reDrawPowerScale();
           CheckReport( checkPos );
           ReleaseDC( fwPowerScale.hWnd, rDC );  
         }

         if ( CheckMode ) {
           if ( midiStatus(hWnd,"ch") == 3 )
             midiRePlay(hWnd,"ch");
           setNextFreq(hWnd);
         }  
         break;
         
    case WM_INITDIALOG:
         CheckOn(hWnd,TRUE);
         LoadFreqScale(hWnd,"freqmax.dat",&CurTab);
         CheckReport( 0 );
         ShowWin( hWnd );
         return TRUE;
         
	case WM_DESTROY:
	     CheckOn(hWnd,FALSE);
         break;    
    }
    return FALSE;
}
