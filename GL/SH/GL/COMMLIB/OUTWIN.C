#include "outwin.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


LRESULT FAR PASCAL __export OutWndProc( HWND hWnd, UINT uMsg,
                               WPARAM wParam, LPARAM lParam );

static OutClassOn = FALSE;

HWND outWin = NULL; // ( == NULL - признак, что окна нет )


static VOID writeout(LPSTR s) {
  if ( outWin == NULL )
    return;
    
  WriteOutBlock( outWin, s, strlen(s) );
}  


int outprint(LPCSTR fmt, ...) {
  va_list marker;
  char    s[256];
  int     size = 256, written = -1;

  va_start( marker, fmt );
  written = _vsnprintf( s, size, fmt, marker );
  if ( written != -1) 
    WriteOutBlock( outWin, s, strlen(s) );

  va_end( marker );
  return written;
}


HWND createOutWin( HWND hWndO, int nCmdShow, 
                short x0, short y0, short dx, short dy ) {
   extern HANDLE ghInst;
   HWND  hOutWnd ;

   WNDCLASS  wndclass ;

   // register tty window class
   if ( !OutClassOn ) {
     wndclass.style =         NULL ;
     wndclass.lpfnWndProc =   OutWndProc ;
     wndclass.cbClsExtra =    0 ;
     wndclass.cbWndExtra =    sizeof( WPARAM ) ;
     wndclass.hInstance =     ghInst ;
     wndclass.hIcon =         LoadIcon(NULL,IDI_APPLICATION); 
                     //LoadIcon( hInstance, MAKEINTRESOURCE( OutICON ) );
     wndclass.hCursor =       LoadCursor( NULL, IDC_ARROW ) ;
     wndclass.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(0,0,0));
       //(COLOR_GRAYTEXT/*WINDOW*/ + 1); //GetStockObject(DKGRAY_BRUSH);
     wndclass.lpszMenuName =  NULL; //MAKEINTRESOURCE( OutMENU ) ;
     wndclass.lpszClassName = gszOutClass ;

     if ( !RegisterClass( &wndclass ) ) 
       return NULL;
     OutClassOn = TRUE;
   }    

   // create the Out window
   hOutWnd = CreateWindow( gszOutClass, gszAppName,
                           WS_OVERLAPPEDWINDOW, 
                           x0,y0,dx,dy,
                           //CW_USEDEFAULT, CW_USEDEFAULT,
                           //CW_USEDEFAULT, CW_USEDEFAULT,
                           hWndO, NULL, ghInst, NULL ) ;

   if (NULL == hOutWnd)
      return ( NULL ) ;

   ShowWindow( hOutWnd, nCmdShow ) ;
   UpdateWindow( hOutWnd ) ;
   
   outWin = hOutWnd;

   return ( hOutWnd ) ;
} 


//---------------------------------------------------------------------------
//  LRESULT FAR PASCAL __export OutWndProc( HWND hWnd, UINT uMsg,
//                                 WPARAM wParam, LPARAM lParam )
//
//  Description:
//     This is the Out Window Proc.  This handles ALL messages
//     to the tty window.
//
//  Parameters:
//     As documented for Window procedures.
//
//---------------------------------------------------------------------------

LRESULT FAR PASCAL __export OutWndProc( HWND hWnd, UINT uMsg,
                               WPARAM wParam, LPARAM lParam )
{
   switch (uMsg)
   {
      case WM_CREATE:
         return ( CreateOutInfo( hWnd ) ) ;

      case WM_COMMAND:
      {
         switch ((WORD) wParam)
         {
            case IDCANCEL:
               PostMessage( hWnd, WM_CLOSE, NULL, 0L ) ;
               break ;
         }
      }
      break ;

      case WM_PAINT:
         PaintOut( hWnd ) ;
         break ;

      case WM_SIZE:
         SizeOut( hWnd, HIWORD( lParam ), LOWORD( lParam ) ) ;
         break ;

      case WM_HSCROLL:
         ScrollOutHorz( hWnd, (WORD) wParam, LOWORD( lParam ) ) ;
         break ;

      case WM_VSCROLL:
         ScrollOutVert( hWnd, (WORD) wParam, LOWORD( lParam ) ) ;
         break ;

      case WM_CHAR:
         ProcessOutCharacter( hWnd, LOBYTE( wParam ) ) ;
         break ;

      case WM_SETFOCUS:
         SetOutFocus( hWnd ) ;
         break ;

      case WM_KILLFOCUS:
         KillOutFocus( hWnd ) ;
         break ;

      case WM_DESTROY:
         DestroyOutInfo( hWnd ) ;
         outWin = NULL;
         //PostQuitMessage( 0 ) ;
         break ;

      /*case WM_CLOSE:
         if (IDOK != MessageBox( hWnd, "OK to close window?", "Out Sample",
                                 MB_ICONQUESTION | MB_OKCANCEL ))
            break ;
      */
         // fall through

      default:
         return( DefWindowProc( hWnd, uMsg, wParam, lParam ) ) ;
   }
   return 0L ;

} // end of OutWndProc()

//---------------------------------------------------------------------------
//  LRESULT  CreateOutInfo( HWND hWnd )
//
//  Description:
//     Creates the tty information structure and sets
//     menu option availability.  Returns -1 if unsuccessful.
//
//  Parameters:
//     HWND  hWnd
//        Handle to main window.
//
//---------------------------------------------------------------------------

LRESULT  CreateOutInfo( HWND hWnd )
{
   //HMENU       hMenu ;
   NPOutINFO   npOutInfo ;

   if (NULL == (npOutInfo =
                   (NPOutINFO) LocalAlloc( LPTR, sizeof( OutINFO ) )))
      return ( (LRESULT) -1 ) ;

   // initialize Out info structure

   CURSORSTATE( npOutInfo )   = CS_HIDE ;
   AUTOWRAP( npOutInfo )      = TRUE ;
   NEWLINE( npOutInfo )       = TRUE ; 
   XSIZE( npOutInfo )         = 0 ;
   YSIZE( npOutInfo )         = 0 ;
   XSCROLL( npOutInfo )       = 0 ;
   YSCROLL( npOutInfo )       = 0 ;
   XOFFSET( npOutInfo )       = 0 ;
   YOFFSET( npOutInfo )       = 0 ;
   COLUMN( npOutInfo )        = 0 ;
   ROW( npOutInfo )           = 0 ;
   HOutFONT( npOutInfo )      = NULL ;
   FGCOLOR( npOutInfo )       = RGB( 0, 0, 0 ) ;

   // clear screen space

   _fmemset( SCREEN( npOutInfo ), ' ', MAXROWS * MAXCOLS ) ;

   // setup default font information

   LFOutFONT( npOutInfo ).lfHeight =        12 ;
   LFOutFONT( npOutInfo ).lfWidth =         10 ;
   LFOutFONT( npOutInfo ).lfEscapement =     0 ;
   LFOutFONT( npOutInfo ).lfOrientation =    0 ;
   LFOutFONT( npOutInfo ).lfWeight =         0 ;
   LFOutFONT( npOutInfo ).lfItalic =         0 ;
   LFOutFONT( npOutInfo ).lfUnderline =      0 ;
   LFOutFONT( npOutInfo ).lfStrikeOut =      0 ;
   LFOutFONT( npOutInfo ).lfCharSet =        OEM_CHARSET ;
   LFOutFONT( npOutInfo ).lfOutPrecision =   OUT_DEFAULT_PRECIS ;
   LFOutFONT( npOutInfo ).lfClipPrecision =  CLIP_DEFAULT_PRECIS ;
   LFOutFONT( npOutInfo ).lfQuality =        DEFAULT_QUALITY ;
   LFOutFONT( npOutInfo ).lfPitchAndFamily = FIXED_PITCH | FF_MODERN ;
   LFOutFONT( npOutInfo ).lfFaceName[0] =    NULL ;

   // set OutInfo handle before any further message processing.

   SetWindowWord( hWnd, GWW_NPOutINFO, (WPARAM) npOutInfo ) ;

   // reset the character information, etc.

   ResetOutScreen( hWnd, npOutInfo ) ;

   //hMenu = GetMenu( hWnd ) ;
   //EnableMenuItem( hMenu, IDM_DISCONNECT,
   //                MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
   //EnableMenuItem( hMenu, IDM_CONNECT, MF_ENABLED | MF_BYCOMMAND ) ;

   return ( (LRESULT) TRUE ) ;

} // end of CreateOutInfo()

//---------------------------------------------------------------------------
//  BOOL  DestroyOutInfo( HWND hWnd )
//
//  Description:
//     Destroys block associated with Out window handle.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//---------------------------------------------------------------------------

BOOL  DestroyOutInfo( HWND hWnd )
{
   NPOutINFO npOutInfo ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   // force connection closed (if not already closed)
   KillOutFocus( hWnd ) ;

   DeleteObject( HOutFONT( npOutInfo ) ) ;

   LocalFree( npOutInfo ) ;
   return ( TRUE ) ;

} // end of DestroyOutInfo()

//---------------------------------------------------------------------------
//  BOOL  ResetOutScreen( HWND hWnd, NPOutINFO npOutInfo )
//
//  Description:
//     Resets the Out character information and causes the
//     screen to resize to update the scroll information.
//
//  Parameters:
//     NPOutINFO  npOutInfo
//        pointer to Out info structure
//
//---------------------------------------------------------------------------

BOOL  ResetOutScreen( HWND hWnd, NPOutINFO npOutInfo )
{
   HDC         hDC ;
   TEXTMETRIC  tm ;
   RECT        rcWindow ;

   if (NULL == npOutInfo)
      return ( FALSE ) ;

   if (NULL != HOutFONT( npOutInfo ))
      DeleteObject( HOutFONT( npOutInfo ) ) ;

   HOutFONT( npOutInfo ) = CreateFontIndirect( &LFOutFONT( npOutInfo ) ) ;

   hDC = GetDC( hWnd ) ;
   SelectObject( hDC, HOutFONT( npOutInfo ) ) ;
   GetTextMetrics( hDC, &tm ) ;
   ReleaseDC( hWnd, hDC ) ;

   XCHAR( npOutInfo ) = tm.tmAveCharWidth  ;
   YCHAR( npOutInfo ) = tm.tmHeight + tm.tmExternalLeading ;

   // a slimy hack to force the scroll position, region to
   // be recalculated based on the new character sizes

   GetWindowRect( hWnd, &rcWindow ) ;
   SendMessage( hWnd, WM_SIZE, SIZENORMAL,
                (LPARAM) MAKELONG( rcWindow.right - rcWindow.left,
                                   rcWindow.bottom - rcWindow.top ) ) ;

   return ( TRUE ) ;

} // end of ResetOutScreen()

//---------------------------------------------------------------------------
//  BOOL  PaintOut( HWND hWnd )
//
//  Description:
//     Paints the rectangle determined by the paint struct of
//     the DC.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window (as always)
//
//---------------------------------------------------------------------------

BOOL  PaintOut( HWND hWnd )
{
   int          nRow, nCol, nEndRow, nEndCol, nCount, nHorzPos, nVertPos ;
   HDC          hDC ;
   HFONT        hOldFont ;
   NPOutINFO    npOutInfo ;
   PAINTSTRUCT  ps ;
   RECT         rect ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   hDC = BeginPaint( hWnd, &ps ) ;
   hOldFont = SelectObject( hDC, HOutFONT( npOutInfo ) ) ;
   SetTextColor( hDC, RGB(0,255,0));//FGCOLOR( npOutInfo ) ) ;
   SetBkColor( hDC, RGB(0,0,0)); //GetSysColor( COLOR_WINDOW ) ) ;
   //SetBkColor( hDC, GetSysColor( COLOR_WINDOW ) ) ;
   rect = ps.rcPaint ;
   nRow =
      min( MAXROWS - 1,
           max( 0, (rect.top + YOFFSET( npOutInfo )) / YCHAR( npOutInfo ) ) ) ;
   nEndRow =
      min( MAXROWS - 1,
           ((rect.bottom + YOFFSET( npOutInfo ) - 1) / YCHAR( npOutInfo ) ) ) ;
   nCol =
      min( MAXCOLS - 1,
           max( 0, (rect.left + XOFFSET( npOutInfo )) / XCHAR( npOutInfo ) ) ) ;
   nEndCol =
      min( MAXCOLS - 1,
           ((rect.right + XOFFSET( npOutInfo ) - 1) / XCHAR( npOutInfo ) ) ) ;
   nCount = nEndCol - nCol + 1 ;
   for (; nRow <= nEndRow; nRow++)
   {
      nVertPos = (nRow * YCHAR( npOutInfo )) - YOFFSET( npOutInfo ) ;
      nHorzPos = (nCol * XCHAR( npOutInfo )) - XOFFSET( npOutInfo ) ;
      rect.top = nVertPos ;
      rect.bottom = nVertPos + YCHAR( npOutInfo ) ;
      rect.left = nHorzPos ;
      rect.right = nHorzPos + XCHAR( npOutInfo ) * nCount ;
      SetBkMode( hDC, OPAQUE ) ;
      ExtTextOut( hDC, nHorzPos, nVertPos, ETO_OPAQUE | ETO_CLIPPED, &rect,
                  (LPSTR)( SCREEN( npOutInfo ) + nRow * MAXCOLS + nCol ),
                  nCount, NULL ) ;
   }
   SelectObject( hDC, hOldFont ) ;
   EndPaint( hWnd, &ps ) ;
   MoveOutCursor( hWnd ) ;
   return ( TRUE ) ;

} // end of PaintOut()

//---------------------------------------------------------------------------
//  BOOL  SizeOut( HWND hWnd, WORD wVertSize, WORD wHorzSize )
//
//  Description:
//     Sizes Out and sets up scrolling regions.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//     WORD wVertSize
//        new vertical size
//
//     WORD wHorzSize
//        new horizontal size
//
//---------------------------------------------------------------------------

BOOL  SizeOut( HWND hWnd, WORD wVertSize, WORD wHorzSize )
{
   int        nScrollAmt ;
   NPOutINFO  npOutInfo ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   YSIZE( npOutInfo ) = (int) wVertSize ;
   YSCROLL( npOutInfo ) = max( 0, (MAXROWS * YCHAR( npOutInfo )) -
                               YSIZE( npOutInfo ) ) ;
   nScrollAmt = min( YSCROLL( npOutInfo ), YOFFSET( npOutInfo ) ) -
                     YOFFSET( npOutInfo ) ;
   ScrollWindow( hWnd, 0, -nScrollAmt, NULL, NULL ) ;

   YOFFSET( npOutInfo ) = YOFFSET( npOutInfo ) + nScrollAmt ;
   SetScrollPos( hWnd, SB_VERT, YOFFSET( npOutInfo ), FALSE ) ;
   SetScrollRange( hWnd, SB_VERT, 0, YSCROLL( npOutInfo ), TRUE ) ;

   XSIZE( npOutInfo ) = (int) wHorzSize ;
   XSCROLL( npOutInfo ) = max( 0, (MAXCOLS * XCHAR( npOutInfo )) -
                                XSIZE( npOutInfo ) ) ;
   nScrollAmt = min( XSCROLL( npOutInfo ), XOFFSET( npOutInfo )) -
                     XOFFSET( npOutInfo ) ;
   ScrollWindow( hWnd, 0, -nScrollAmt, NULL, NULL ) ;
   XOFFSET( npOutInfo ) = XOFFSET( npOutInfo ) + nScrollAmt ;
   SetScrollPos( hWnd, SB_HORZ, XOFFSET( npOutInfo ), FALSE ) ;
   SetScrollRange( hWnd, SB_HORZ, 0, XSCROLL( npOutInfo ), TRUE ) ;

   InvalidateRect( hWnd, NULL, TRUE ) ;

   return ( TRUE ) ;

} // end of SizeOut()

//---------------------------------------------------------------------------
//  BOOL  ScrollOutVert( HWND hWnd, WORD wScrollCmd, WORD wScrollPos )
//
//  Description:
//     Scrolls Out window vertically.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//     WORD wScrollCmd
//        type of scrolling we're doing
//
//     WORD wScrollPos
//        scroll position
//
//---------------------------------------------------------------------------

BOOL  ScrollOutVert( HWND hWnd, WORD wScrollCmd, WORD wScrollPos )
{
   int        nScrollAmt ;
   NPOutINFO  npOutInfo ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   switch (wScrollCmd)
   {
      case SB_TOP:
         nScrollAmt = -YOFFSET( npOutInfo ) ;
         break ;

      case SB_BOTTOM:
         nScrollAmt = YSCROLL( npOutInfo ) - YOFFSET( npOutInfo ) ;
         break ;

      case SB_PAGEUP:
         nScrollAmt = -YSIZE( npOutInfo ) ;
         break ;

      case SB_PAGEDOWN:
         nScrollAmt = YSIZE( npOutInfo ) ;
         break ;

      case SB_LINEUP:
         nScrollAmt = -YCHAR( npOutInfo ) ;
         break ;

      case SB_LINEDOWN:
         nScrollAmt = YCHAR( npOutInfo ) ;
         break ;

      case SB_THUMBPOSITION:
         nScrollAmt = wScrollPos - YOFFSET( npOutInfo ) ;
         break ;

      default:
         return ( FALSE ) ;
   }
   if ((YOFFSET( npOutInfo ) + nScrollAmt) > YSCROLL( npOutInfo ))
      nScrollAmt = YSCROLL( npOutInfo ) - YOFFSET( npOutInfo ) ;
   if ((YOFFSET( npOutInfo ) + nScrollAmt) < 0)
      nScrollAmt = -YOFFSET( npOutInfo ) ;
   ScrollWindow( hWnd, 0, -nScrollAmt, NULL, NULL ) ;
   YOFFSET( npOutInfo ) = YOFFSET( npOutInfo ) + nScrollAmt ;
   SetScrollPos( hWnd, SB_VERT, YOFFSET( npOutInfo ), TRUE ) ;

   return ( TRUE ) ;

} // end of ScrollOutVert()

//---------------------------------------------------------------------------
//  BOOL  ScrollOutHorz( HWND hWnd, WORD wScrollCmd, WORD wScrollPos )
//
//  Description:
//     Scrolls Out window horizontally.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//     WORD wScrollCmd
//        type of scrolling we're doing
//
//     WORD wScrollPos
//        scroll position
//
//---------------------------------------------------------------------------

BOOL  ScrollOutHorz( HWND hWnd, WORD wScrollCmd, WORD wScrollPos )
{
   int        nScrollAmt ;
   NPOutINFO  npOutInfo ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   switch (wScrollCmd)
   {
      case SB_TOP:
         nScrollAmt = -XOFFSET( npOutInfo ) ;
         break ;

      case SB_BOTTOM:
         nScrollAmt = XSCROLL( npOutInfo ) - XOFFSET( npOutInfo ) ;
         break ;

      case SB_PAGEUP:
         nScrollAmt = -XSIZE( npOutInfo ) ;
         break ;

      case SB_PAGEDOWN:
         nScrollAmt = XSIZE( npOutInfo ) ;
         break ;

      case SB_LINEUP:
         nScrollAmt = -XCHAR( npOutInfo ) ;
         break ;

      case SB_LINEDOWN:
         nScrollAmt = XCHAR( npOutInfo ) ;
         break ;

      case SB_THUMBPOSITION:
         nScrollAmt = wScrollPos - XOFFSET( npOutInfo ) ;
         break ;

      default:
         return ( FALSE ) ;
   }
   if ((XOFFSET( npOutInfo ) + nScrollAmt) > XSCROLL( npOutInfo ))
      nScrollAmt = XSCROLL( npOutInfo ) - XOFFSET( npOutInfo ) ;
   if ((XOFFSET( npOutInfo ) + nScrollAmt) < 0)
      nScrollAmt = -XOFFSET( npOutInfo ) ;
   ScrollWindow( hWnd, -nScrollAmt, 0, NULL, NULL ) ;
   XOFFSET( npOutInfo ) = XOFFSET( npOutInfo ) + nScrollAmt ;
   SetScrollPos( hWnd, SB_HORZ, XOFFSET( npOutInfo ), TRUE ) ;

   return ( TRUE ) ;

} // end of ScrollOutHorz()

//---------------------------------------------------------------------------
//  BOOL  SetOutFocus( HWND hWnd )
//
//  Description:
//     Sets the focus to the Out window also creates caret.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//---------------------------------------------------------------------------

BOOL  SetOutFocus( HWND hWnd )
{
   NPOutINFO  npOutInfo ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;
   if ((CURSORSTATE( npOutInfo ) != CS_SHOW)) {

      CreateCaret( hWnd, NULL, XCHAR( npOutInfo ), YCHAR( npOutInfo ) ) ;
      ShowCaret( hWnd ) ;
      CURSORSTATE( npOutInfo ) = CS_SHOW ;
   }
   MoveOutCursor( hWnd ) ;
   return ( TRUE ) ;

} // end of SetOutFocus()

//---------------------------------------------------------------------------
//  BOOL  KillOutFocus( HWND hWnd )
//
//  Description:
//     Kills Out focus and destroys the caret.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//---------------------------------------------------------------------------

BOOL  KillOutFocus( HWND hWnd )
{
   NPOutINFO  npOutInfo ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   if ((CURSORSTATE( npOutInfo ) != CS_HIDE))
   {
      HideCaret( hWnd ) ;
      DestroyCaret() ;
      CURSORSTATE( npOutInfo ) = CS_HIDE ;
   }
   return ( TRUE ) ;

} // end of KillOutFocus()

//---------------------------------------------------------------------------
//  BOOL  MoveOutCursor( HWND hWnd )
//
//  Description:
//     Moves caret to current position.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//---------------------------------------------------------------------------

BOOL  MoveOutCursor( HWND hWnd )
{
   NPOutINFO  npOutInfo ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   if ((CURSORSTATE( npOutInfo ) & CS_SHOW))
      SetCaretPos( (COLUMN( npOutInfo ) * XCHAR( npOutInfo )) -
                   XOFFSET( npOutInfo ),
                   (ROW( npOutInfo ) * YCHAR( npOutInfo )) -
                   YOFFSET( npOutInfo ) ) ;

   return ( TRUE ) ;

} // end of MoveOutCursor()


BOOL  ProcessOutCharacter( HWND hWnd, BYTE bOut ) {

  WriteOutBlock( hWnd, &bOut, 1 ) ;
  return TRUE;
}  


//---------------------------------------------------------------------------
//  BOOL  CloseConnection( HWND hWnd )
//
//  Description:
//     Closes the connection to the port.  Resets the connect flag
//     in the OutINFO struct.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//---------------------------------------------------------------------------
/*
BOOL  CloseConnection( HWND hWnd )
{

   KillOutFocus( hWnd ) ;

   // change the selectable items in the menu

   // hMenu = GetMenu( hWnd ) ;
   //EnableMenuItem( hMenu, IDM_DISCONNECT,
   //                MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
   //EnableMenuItem( hMenu, IDM_CONNECT,
   //                MF_ENABLED | MF_BYCOMMAND ) ;

   return ( TRUE ) ;

} // end of CloseConnection()
*/

//---------------------------------------------------------------------------
//  BOOL  WriteOutBlock( HWND hWnd, LPSTR lpBlock, int nLength )
//
//  Description:
//     Writes block to Out screen.  Nothing fancy - just
//     straight Out.
//
//  Parameters:
//     HWND hWnd
//        handle to Out window
//
//     LPSTR lpBlock
//        far pointer to block of data
//
//     int nLength
//        length of block
//
//---------------------------------------------------------------------------

BOOL  WriteOutBlock( HWND hWnd, LPSTR lpBlock, int nLength )
{
   int        i ;
   NPOutINFO  npOutInfo ;
   RECT       rect ;

   if (NULL == (npOutInfo = (NPOutINFO) GetWindowWord( hWnd, GWW_NPOutINFO )))
      return ( FALSE ) ;

   for (i = 0 ; i < nLength; i++)
   {
      switch (lpBlock[ i ])
      {
         case ASCII_BEL:
            // Bell
            MessageBeep( 0 ) ;
            break ;

         case ASCII_BS:
            // Backspace
            if (COLUMN( npOutInfo ) > 0)
               COLUMN( npOutInfo ) -- ;
            MoveOutCursor( hWnd ) ;
            break ;

         case ASCII_CR:
            // Carriage return
            COLUMN( npOutInfo ) = 0 ;
            MoveOutCursor( hWnd ) ;
            if (!NEWLINE( npOutInfo ))
               break;

            // fall through

         case ASCII_LF:
            // Line feed
            if (ROW( npOutInfo )++ == MAXROWS - 1)
            {
               _fmemmove( (LPSTR) (SCREEN( npOutInfo )),
                          (LPSTR) (SCREEN( npOutInfo ) + MAXCOLS),
                          (MAXROWS - 1) * MAXCOLS ) ;
               _fmemset( (LPSTR) (SCREEN( npOutInfo ) + (MAXROWS - 1) * MAXCOLS),
                         ' ', MAXCOLS ) ;
               InvalidateRect( hWnd, NULL, FALSE ) ;
               ROW( npOutInfo )-- ;
            }
            MoveOutCursor( hWnd ) ;
            break ;

         default:
            *(SCREEN( npOutInfo ) + ROW( npOutInfo ) * MAXCOLS +
                COLUMN( npOutInfo )) = lpBlock[ i ] ;
            rect.left = (COLUMN( npOutInfo ) * XCHAR( npOutInfo )) -
                        XOFFSET( npOutInfo ) ;
            rect.right = rect.left + XCHAR( npOutInfo ) ;
            rect.top = (ROW( npOutInfo ) * YCHAR( npOutInfo )) -
                       YOFFSET( npOutInfo ) ;
            rect.bottom = rect.top + YCHAR( npOutInfo ) ;
            InvalidateRect( hWnd, &rect, FALSE ) ;

            // Line wrap
            if (COLUMN( npOutInfo ) < MAXCOLS - 1)
               COLUMN( npOutInfo )++ ;
            else if (AUTOWRAP( npOutInfo ))
               WriteOutBlock( hWnd, "\r\n", 2 ) ;
            break;
      }
   }
   return ( TRUE ) ;

} // end of WriteOutBlock()

