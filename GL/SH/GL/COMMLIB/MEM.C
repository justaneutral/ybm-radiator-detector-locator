#include "cinc.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

BOOL error = FALSE;

HINSTANCE ghInst = NULL;

// сторка для запоминания сообщения об ошибке

#define merrorstring 64 // максимальная длина сообщения об ошибке

static char errorstringmessage[merrorstring] = ""; 

VOID errorstring( char *s ) {
// запоминание сообщения об ошибке
// для случая, когда неизвестен hWnd.

  strcpy(errorstringmessage, s);
  error = TRUE;
}


//VOID message ( HWND hWnd, LPCSTR s ) {
// message Box.

    
//} 
int message(HWND hDlg, char *fmt, ...) {
//   hWnd - порождающее окно,
//   fmt  - текст.

  va_list marker;
  char    buffer[256];
  int     size = 256, written = -1;

  if ( error )
    return 0;

  MessageBeep(MB_ICONHAND); 

  va_start( marker, fmt );
  written = _vsnprintf( buffer, size, fmt, marker );
  if ( written != -1) 
    MessageBox(hDlg, buffer, "", MB_OK | MB_ICONEXCLAMATION);
  
    //SendDlgItemMessage(hDlg,ne,WM_SETTEXT,0,(LPARAM)(LPCSTR)buffer);

  va_end( marker );
  return written;
}




BOOL checkError ( HWND hWnd ) {
// проверка наличия ошибок и выдача запомненных сообщений
static BOOL errorWindow = FALSE;

  if ( errorstringmessage[0] != 0 ) {
    if ( errorWindow )
      return error;

    errorWindow = TRUE;
    MessageBeep( MB_ICONEXCLAMATION );
    MessageBox ( hWnd, errorstringmessage,
       "Fatal Error (2)", MB_OK | MB_ICONSTOP );
    errorWindow = FALSE;
    errorstringmessage[0] = 0;
  }
  return error;
}


VOID errormessage ( HWND hWnd, LPCSTR s ) {
// message Box и установка ошибки.

//   hWnd - порождающее окно,
//   s    - текст.
       
  if ( error )
    return;

  error = TRUE;

  MessageBeep(MB_ICONEXCLAMATION); 

  MessageBox ( hWnd, s, "Fatal Error:", MB_OK | MB_ICONSTOP );
} 


int myDialogBox( HWND hWnd, LPCSTR lpszTemplate, FARPROC lpDlgProc)

//  My Dialog Box.

//   hWnd         - порождающее окно,
//   lpszTemplate - строка - имя диалога в RC,
//   lpDlgProc    - оконная процедура.

{
  FARPROC     fpfn;
  int i;
    
  if ( error )
    return 0;
    
  fpfn = MakeProcInstance(lpDlgProc, ghInst);
  i = DialogBox(ghInst, lpszTemplate, hWnd, (DLGPROC)fpfn);
  FreeProcInstance(fpfn);
        
  return i;
}

LPGLOBALARRAY getMemDC( VOID FAR *p ) {
// возвращает дескриптор глобального массива.

  return (LPGLOBALARRAY)(((LPCSTR)p)-sizeof(GLOBALARRAY));
}  

BOOL memOn(HWND hWnd, VOID FAR **po, long l, BOOL St) {

// My Global Alloc & Global Free.
// l - число байт.
// St - On / Off. (TRUE / FALSE).

  VOID FAR *p;
  HGLOBAL h;
  LPGLOBALARRAY lpg;
  
  if ( error )
    return FALSE;
  
  if ( St ) {
    /*
    if ( l >= 1024L*64L ) {
      errormessage(hWnd, "Data GT 64K");
      return FALSE;
    }  
    */
    
    if ( (h = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, l+sizeof(GLOBALARRAY) ) ) == NULL ) {
      errormessage(hWnd, "Error GlobalAllock Memory");
      return FALSE;
    }  

    p = GlobalLock(h);
    
    lpg = (LPGLOBALARRAY)p;
    
    if ( p == NULL ) {
      h = GlobalFree(h);
      errormessage(hWnd, "Error GlobalLock Memory");
      return FALSE;
    }
  
    lpg->h = h;
    lpg->m = 0;
    lpg->l = l;
  
    *po = (VOID FAR *)(((LPCSTR)p)+sizeof(GLOBALARRAY));
    return TRUE;
  }
  else {  
    if ( po == NULL )
      return TRUE;
  
    h = getMemDC(*po)->h;
  
    if ( GlobalUnlock(h) != 0 ) {
      errormessage(hWnd,"Error GlobalUnlock Memory");
      return FALSE;
    }

    if ( GlobalFree(h) != 0 ) {
      errormessage(hWnd,"Error GlobalFree Memory");
      return FALSE;
    }
      
    *po = NULL;
      
    return TRUE;
  }  
} /* memOn */


BOOL doubleOn(HWND hWnd, DOUBLEARRAY *da, short m, BOOL St) {

// размещение/удаление массива double в глобальной памяти.
// m - число элементов в массиве.
// St - On / Off. (TRUE / FALSE).

  DOUBLEARRAY p;
  double huge *u;
  short i;
  
  if ( error )
    return FALSE;

  if ( St ) {
    if ( error )
      return FALSE;
    
    if ( !memOn(hWnd, &p, ((long)m)*sizeof(double), TRUE ) )
      return FALSE;
    
    getMemDC(p)->m = m;
  
    u = p;
    
    for ( i = 0; i < m; i++ ) {
      u[i] = 0.0;
    }  
  
    *da = p;
    return TRUE;
  }  
  else {
    return memOn(hWnd, da, m*sizeof(double), FALSE );
  } 
}  


BOOL readMem(HWND hWnd, HFILE hfFile, VOID FAR *ga) {
// чтение из файла в существующий глобальный массив
  long l;
  GLOBALARRAY dc;
  
  if ( error )
    return FALSE;

  l = getMemDC(ga)->l;
  
  if ( _hread(hfFile, &dc, sizeof(dc)) != sizeof(dc) ) {
    errormessage ( hWnd, "File Read Error" );
    _lclose ( hfFile );
    return FALSE;
  }
  
  if ( dc.l != l ) {
    errormessage(hWnd,"Object Length Read Error");
    _lclose(hfFile);
    return FALSE;
  }
  

  if ( _hread(hfFile, ga, l) != l ) {
    errormessage(hWnd,"File Read Error");
    _lclose(hfFile);
    return FALSE;
  }
  
  return TRUE;
}   


BOOL openMem(HWND hWnd, HFILE hfFile, VOID FAR **ga) {
// чтение из файла в создаваемый глобальный массива
  long l;
  GLOBALARRAY dc;
  
  if ( error )
    return FALSE;
  
  if ( _hread(hfFile, &dc, sizeof(dc)) != sizeof(dc) ) {
    errormessage ( hWnd, "File Read Error" );
    _lclose ( hfFile );
    return FALSE;
  }

  l = dc.l;
  
  if ( !memOn ( hWnd, ga, l, TRUE ) ) {
    _lclose ( hfFile );
    return FALSE;
  }
  
  if ( _hread ( hfFile, *ga, l ) != l ) {
    errormessage ( hWnd, "File Read Error" );
    _lclose ( hfFile );
    return FALSE;
  }
  
  return TRUE;
}


BOOL saveMem(HWND hWnd, HFILE hfFile, VOID FAR *ga) {
// запись глобального массива в файл
  long l;
  LPGLOBALARRAY p;
  
  if ( error )
    return FALSE;

  p = getMemDC(ga);
  l = p->l+sizeof(GLOBALARRAY);
  
  if ( _hwrite(hfFile, p, l != l ) ) {
    message ( hWnd, "File Write Error" );
    _lclose ( hfFile );
    return FALSE;
  }
  return TRUE;
}   

 