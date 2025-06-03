#include "cinc.h"
#include "math.h"
#include "conio.h"
#include "port.h"

#define NSAMPLE_IN   3100 
#define NGR 	     2500

//   множитель для перевода отсчетов в метры.

#define SB_FREQ 4
#define WSound ((331.0*1.03)/(11025.0*SB_FREQ))


FUNWIN fwAmp,fwStr,fwDir,fwPro;

#define mDist 4

static long Dist[ mDist ] = { 0, 0, 0, 0 },
//     LeftRightD = 11,
     NOfSamples = 0;

static DOUBLEARRAY
  pro   = NULL,
  dStr  = NULL,
  gra   = NULL,
  
  rc0   = NULL,     // AutoFilt
  xemm  = NULL,
  sr0   = NULL,
  
  ampS [ mDist ] = { NULL, NULL, NULL, NULL };

static WaveBuffer pb;

static BOOL TestOn = FALSE,
     WaveMode = FALSE;

static int  SideTest = 0;

static double mediana = 0.0;

double cosA = 0.0;


//#define mSilence (538*SB_FREQ)    // тихий кусок
#define mSilence (134*SB_FREQ)    // тихий кусок

#define LocDiap (55*SB_FREQ)

short MaxDist = (mSilence-LocDiap);


static VOID ClearMid( HWND hWnd ) {
  int i;
  
  loop( i, mDist )
    Dist[i] = 0;
  
  NOfSamples = 0;
  
  printW(hWnd,IDC_SAMPLE,"%ld",NOfSamples);

  loop( i, mDist )
    arrEquC(ampS[i], 0.0);
}
  

//------------------

VOID GanteliaOn(HWND hWnd, BOOL St) {
  int i;
  
  if ( CommonError ) 
    return;

  DoubleOn(hWnd, NSAMPLE_IN, &sl, St);
  DoubleOn(hWnd, NSAMPLE_IN, &sr, St);
  
  DoubleOn(hWnd, NGR, &pro, St);
  DoubleOn(hWnd, NGR, &dStr, St);      // S

  DoubleOn(hWnd, map, &rc0, St);       // AutoFilt
  DoubleOn(hWnd, mSilence, &xemm, St);
  DoubleOn(hWnd, mSilence, &sr0, St);

  loop( i, mDist )
    DoubleOn(hWnd, NGR, &ampS[i], St);  
  
  DoubleOn(hWnd, NGR, &gra, St);

  if ( CommonError ) {
    return;
  }  

  TestOn = FALSE;
  SideTest = 0;
  ColorsOn(St);
  
  WaveOn(hWnd,St,SB_FREQ);

  if ( St ) {
	initXemm( xemm );
    InitWaveWindow();
    OutPort(0);
    OpenBuffer(hWnd,NSAMPLE_IN*4L,77,&pb);
    InitGraWin(&fwAmp,gra,"A");
    InitGraWin(&fwStr,dStr,"S");
    InitGraWin(&fwPro,pro,"Pro");
    InitDirWin(&fwDir,"D");
  }
}


VOID NextSide( void ) {
  if ( SideTest >= mDist-1 ) 
    SideTest = 0;
  else 
    SideTest++;
}


VOID StartStep( HWND hWnd ) {

  if (WaveMode or CommonError) 
    return;
  
  WaveMode = TRUE;

  WaveInp(hWnd,&pb);
  delay(300);
  OutPort(SideTest+1);
}


short pauD(DOUBLEARRAY d) {
  short i,im,m;
  double fm,fa;
  
  m = GetMemDC(d)->m;
  
  im = absD(d);
  fm = fabs(d[im])*0.01;
  
  for ( i = 1; i < m; i++ ) {
    fa = fabs(d[i]);
    if ( fa > fm ) {
      return i;
    }
  }

  return 1;  
}

short GetDist0( DOUBLEARRAY ampM ) {

  short  i,iD,im;
  double s,sm,siSq,siSq0,alpha, fMin = 0.0000001;
  short j;

  s = 0.0;
  
  loop ( i, mSilence ) 
    s += sqr( ampM[i] );

  siSq = s/mSilence*1.4;
  
  im = absD( ampM );
  sm = sqr( ampM[im]*0.05 );
  
  siSq0 = siSq;
  
  if ( siSq0 < sm )
    siSq0 = sm;
  
  if (siSq <= fMin)
    return 0;
  
  s = 0.0;
  
  loop ( i, NGR ) {
    s = s-0.5*log((2.0*3.1415926*siSq))-sqr(ampM[i])/(2.0*siSq);
    pro[i] = s;
  }

  s = 0.0;
  
  loop ( i, NGR ) {
    j = NGR-i-1;
    
    pro[j] += s;

    if ( j > (mSilence+200*SB_FREQ) ) {
      alpha = 2.0;
      s = s-0.5*log((2.0*3.1415926*siSq*alpha))-sqr(ampM[j])/(2.0*siSq*alpha);
    }  
    else {
      if (j < mSilence)
        alpha = 333.0;
      else {
        alpha = (j-mSilence)/(200.0*SB_FREQ);
        alpha = 150.0*(1.0-alpha)+2.0*alpha;
      }  
      s = s-0.5*log((2.0*3.1415926*siSq0*alpha))-sqr(ampM[j])/(2.0*siSq0*alpha);
    }
  }

  iD = maxD(pro);
  
  iD -= (mSilence+5*SB_FREQ);   
  
  if ( iD < 0 ) 
    iD = 0;
  
  iD += mSilence;
  
  if (iD > (NGR-LocDiap))
    iD = NGR-LocDiap;
    
  return iD;
}


short GetDist(HWND hWnd, DOUBLEARRAY ampM ) {

  short  i,iD,im;
  double s, siSq, siSq0, sm, alpha, fMin = 0.0000001;
  short j;

  arrEqu( gra, ampM );
  
  if ( IsDlgButtonChecked( hWnd, IDC_CLEAR_MODE ) )
  {
  {
  short ti,js,cl;
  
  cl = mSilence;
  
  if ( MaxDist > 80 ) { 
  loop ( i, LocDiap ) {
    ti = i+MaxDist;
    
    j = 1;
    
    while ((ampM[ti]*ampM[ti-j] > 0.0) and (j < 40) )
      j++;
      
    j = 40;  
    ti -= j;  
      
    js = ti-140;
    
    if (js <= mSilence) 
      js = mSilence;
      
    s = 0.0;
    loop2 ( j, js, ti) {
      if (fabs(ampM[j]) > s)
        s = fabs(ampM[j]);
    }
    if ( (s*3.0) < fabs(ampM[i+MaxDist]) ) {
      if ((cl >= js) and (cl < ti))
        js = cl;
        
      cl = ti;  
      loop2 (j,js,cl)
        gra[j] = 0.0;  
    }
    }
  }
  }
  }
  s = 0.0;
  
  loop ( i, mSilence ) 
    s += sqr( gra[i] );

  siSq = s/mSilence*1.2;
  
  im = absD( gra );
  sm = sqr( gra[im]*0.05 );
  
  siSq0 = siSq;
  
  if ( siSq0 < sm )
    siSq0 = sm;

  if (siSq <= fMin)
    return 0;
  
  s = 0.0;
  
  arrEquC(pro,0.0);
  
  loop ( i, LocDiap ) {
    j = i+MaxDist;
    s = s-0.5*log((2.0*3.1415926*siSq))-sqr(gra[j])/(2.0*siSq);
    pro[j] = s;
  }

  s = 0.0;
  
  loop ( i, LocDiap ) {
    j = MaxDist+LocDiap-i-1;
    pro[j] += s;

    if ( j > (mSilence+200.0*SB_FREQ) ) {
      alpha = 1.2;
      s = s-0.5*log((2.0*3.1415926*siSq*alpha))-sqr(gra[j])/(2.0*siSq*alpha);
    }  
    else {
    
      if (j < mSilence)
        alpha = 333.0;
      else {
        alpha = (j-mSilence)/(200.0*SB_FREQ);
        alpha = 150.0*(1.0-alpha)+1.2*alpha;
      }
      s = s-0.5*log((2.0*3.1415926*siSq0*alpha))-sqr(gra[j])/(2.0*siSq0*alpha);
    }
  }
  
  s = pro[MaxDist];
  
  loop ( i, LocDiap ) {
    j = MaxDist+LocDiap-i-1;
    if ( pro[j] < s ) {
      pro[j] = 0.0;
    }  
    else 
      pro[j] -= s;
  }

  iD = maxD(pro);
  
  iD -= (mSilence+2*SB_FREQ);   
  
  if ( iD < 0 ) 
    iD = 0;

  return iD;
}


BOOL AutoFilt( short i0 )
{
  short i,j;
  double s,aa[map+1];
  
  if ( CommonError )
    return FALSE;

  if ( i0 < mSilence )
    return FALSE;
    
  lowFilter(sr);  
    
  loop(i,mSilence)
    sr0[i] = sr[i0-mSilence+i];
    
  arrMul(sr0,sr0,xemm);  

  corr(sr0,rc0);
  
  if ( not autoc(aa,rc0) )
    return FALSE;
    
  
  loop2 (i,map,NSAMPLE_IN) {
    s = 0.0;
    loop (j, map)
      s += aa[j+1]*sr[i-j];
    
    sr[i-map] = s;
  }
  
  loop2 (i,map,NSAMPLE_IN)
    sr[NSAMPLE_IN-i+map-1] = sr[NSAMPLE_IN-i];
    
  return TRUE;  
}

short AddMidAmp(HWND hWnd, DOUBLEARRAY ampM, short i0 ) {

  short  i;
  double s;
  
  if ( not AutoFilt(i0+mSilence) )
    return 0;

  s = 0.0;
  
  loop( i, i0+mSilence )
    s += sr[i]*sr[i];

  if ( s < 1.0 )
   s = 1.0;
   
  s = 100.0/s;
 if ( IsDlgButtonChecked( hWnd, IDC_CLEAR_BUTTON ) ){  
  for ( i = i0; (i < NSAMPLE_IN) && ((i-i0) < NGR); i++ ) 
    ampM[ i-i0 ] = 0.85*ampM[ i-i0 ]+0.15*sr [ i ]*s;
 }
 else {
  for ( i = i0; (i < NSAMPLE_IN) && ((i-i0) < NGR); i++ ) 
    ampM[ i-i0 ] += sr [ i ]*s;
 }   

  return GetDist( hWnd,ampM );
}

extern double r[4];

BOOL DrawDirs( void ) {
  short i;
  extern VOID rsolv( void );
    
  loop(i,4)
    r[i] = (Dist[i]*WSound*100.0)+0.54;
    
  rsolv();  

  if ( fwDir.On )  
    SendMessage( fwDir.hWnd, WM_COMMAND, 100,0 );
    
  return TRUE;
}


extern double v[4];

VOID DataWork( HWND hWnd ) {
  short i;

  trend(sl);
  
  i = pauD(sl);
  
  printW(hWnd,IDC_SIL,"%d",i);
  
  if ( i < mSilence ) 
    return;
  
  i -= mSilence;

  Dist[SideTest] = AddMidAmp(hWnd,ampS[SideTest],i);
  
  if ( SideTest >= mDist-1 ) {
    if ( DrawDirs() )

    printW(hWnd,IDC_DISTANCE,
      "d ( %5.0f, %5.0f, %5.0f, %5.0f), s (%5.0f, %5.0f, %5.0f), %6.0f.",
      r[0],
      r[1],
      r[2],
      r[3],
      v[0],v[1],v[2],v[3]);  
      
    else

    printW(hWnd,IDC_DISTANCE,"Distance: ");

    NOfSamples++;
    
    printW(hWnd,IDC_SAMPLE,"%ld",NOfSamples);
    
    if ( fwDir.On )  
      SendMessage( fwDir.hWnd, WM_COMMAND, 102,0 );

  //  if ( IsDlgButtonChecked( hWnd, IDC_CLEAR_BUTTON ) )
  //    if (NOfSamples > 5)
  //      ClearMid( hWnd );
  }

  if ( fwAmp.On )  
    SendMessage( fwAmp.hWnd, WM_COMMAND, 100,0 );
  
  if ( fwPro.On )  
    SendMessage( fwPro.hWnd, WM_COMMAND, 100,0 );

  if ( fwStr.On ) {
    arrEquC(dStr,0.0);
    loop (i,4) {
      arrEqu(gra,ampS[i]);
      lowFilter(gra);
      arrAbs(gra,gra);
      arrAdd(dStr,dStr,gra);
    }
    
    MaxDist = GetDist0(dStr);
    
    SendMessage( fwStr.hWnd, WM_COMMAND, 100,0 );
  }
  
  NextSide();
}

VOID ShoWin( HWND hWnd ) {
  MakeWaveWin(hWnd);
  MakeGraWin(hWnd,&fwAmp,1);
  MakeGraWin(hWnd,&fwPro,2);
  MakeGraWin(hWnd,&fwStr,3);

  MakeDirWin(hWnd,&fwDir);
}


BOOL FAR PASCAL __export GantDialogProc(HWND hWnd, unsigned msg, WORD wParam, LONG lParam)
{   

  if ( CommonError ) {
    if ( msg == WM_COMMAND ) {
      if ( (wParam == IDOK) or (wParam == IDCANCEL) )
        EndDialog(hWnd,1) ;
    }    
    return FALSE ;
  }  
  
  switch (msg) {
    case WM_COMMAND:
      switch (wParam) {
      
        case IDOK:
        case IDCANCEL:
            EndDialog(hWnd,1) ;
            break;

        case IDC_MODE:
            EndDialog(hWnd,2) ;
            break;
            
        case IDC_WIN:
            ShoWin(hWnd);
            //OutPort();
    	    break;
    	    
        case IDC_CLEAR:
            ClearMid( hWnd );    	
    	    break;

        case IDC_TEST:
            if (!TestOn)
              StartStep(hWnd);    	
    	    break;

    	case IDC_START:
            if (!TestOn) {
              TestOn = TRUE;
            //SideTest = 1;
              StartStep(hWnd);
            }  
            break;
            
        case IDC_STOP:
            TestOn = FALSE;
            //SideTest = 1;
  	        break;
      }
      break;
         
    case MM_WIM_DATA: 		// End Of Record.        
         unprepareInp(hWnd,&pb);
         CloseDevice(hWnd,TRUE);
         WaveMode = FALSE;
         /*
         if ( SideTest == 1 )
           midiStop(hWnd,"chl");
         else
           midiStop(hWnd,"chr");
         */  
         CopyWave(&pb);
         if ( waveWin.On )
           SendMessage(waveWin.hWnd, WM_COMMAND,100,0);
         DataWork(hWnd);
         if ( TestOn )
           StartStep( hWnd );
         break;

    case MM_WOM_DONE:
         unprepareOut(hWnd,&pb);
         CloseDevice(hWnd,FALSE);
         break;
         
    case WM_INITDIALOG:
         GanteliaOn(hWnd,TRUE);
         ShoWin(hWnd);
         return TRUE;
         
	case WM_DESTROY:
	     GanteliaOn(hWnd,FALSE);
         break;    
    }
    return FALSE;
}

