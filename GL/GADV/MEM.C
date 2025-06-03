#include "cinc.h"

BOOL CommonError = FALSE;


VOID Message(HWND Window, LPCSTR s) {
// Message Box.

//   hWnd - порождающее окно,
//   s    - текст.

  MessageBox(Window, s, "", MB_OK);
} /* Message */


int myDialogBox( HWND hWnd, LPCSTR lpszTemplate, FARPROC lpDlgProc)

//  My Dialog Box.

//   hWnd         - порождающее окно,
//   lpszTemplate - строка - имя диалога в RC,
//   lpDlgProc    - оконная процедура.

{
  extern HANDLE ghInst;   // app instance handle
  FARPROC     fpfn;
  int i;
    
  fpfn = MakeProcInstance(lpDlgProc, ghInst);
  i = DialogBox(ghInst, lpszTemplate, hWnd, (DLGPROC)fpfn);
  FreeProcInstance(fpfn);
        
  return i;
}

LPGLOBALARRAY GetMemDC( VOID FAR *p ) {
// возвращает дескриптор глобального массива.

  return (LPGLOBALARRAY)(((LPCSTR)p)-sizeof(GLOBALARRAY));
}  

BOOL MemoryOn(VOID FAR **po, long l, BOOL St) {

// My Global Alloc & Global Free.
// l - число байт.
// St - On / Off. (TRUE / FALSE).

  VOID FAR *p;
  HANDLE h;
  
  if ( CommonError )
    return FALSE;

  if ( St ) {
    if ( l <= 0 ) {
      CommonError = TRUE;
      return FALSE;
    }  
  
    if ( (h = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, l+sizeof(GLOBALARRAY))) == NULL ) {
      CommonError = TRUE;
      return FALSE;
    }
    
    p = GlobalLock(h);
    if ( p == NULL ) {
      CommonError = TRUE;
      h = GlobalFree(h);
      return FALSE;
    }
  
    ((LPGLOBALARRAY)p)->h = h;
    ((LPGLOBALARRAY)p)->m = 0;
  
    *po = (VOID FAR *)(((LPCSTR)p)+sizeof(GLOBALARRAY));
    return TRUE;
  }
  else {  
    if ( po == NULL )
      return TRUE;
  
    h = GetMemDC(po)->h;
  
    if ( GlobalUnlock(h) != 0 )
      return FALSE;
    if ( GlobalFree(h) != 0 ) {
      CommonError = TRUE;    
      return FALSE;
    }  
    return TRUE;
  }  
} /* MemoryOn */


BOOL DoubleOn(HWND hWnd, short m, DOUBLEARRAY *da, BOOL St) {

// размещение/удаление массива double в глобальной памяти.
// m - число элементов в массиве.
// St - On / Off. (TRUE / FALSE).

  VOID FAR *p;
  short i;
  DOUBLEARRAY a;
  
  if ( St ) {
    if ( CommonError )
      return FALSE;
    
    if ( !MemoryOn( &p, m*(long)sizeof(double), TRUE ) ) {
      Message(hWnd,"Not enough memory");
      CommonError = TRUE;
      return FALSE;
    }
  
    a = (DOUBLEARRAY)p;
    
    *da = a;

    GetMemDC(a)->m = m;
  
    for ( i = 0; i < m; i++ )
      a[i] = 0.0;

    return TRUE;
  }  
  else {
    p = da;
    return MemoryOn( &p, m*(long)sizeof(double), FALSE );
  } 
}  
 