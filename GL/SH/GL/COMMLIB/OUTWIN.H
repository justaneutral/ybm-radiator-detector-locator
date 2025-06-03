//---------------------------------------------------------------------------
//
//  Module: tty.h
//
//  Purpose:
//     This is the header file for the Out sample.
//
//---------------------------------------------------------------------------
//
//  Written by Microsoft Product Support Services, Windows Developer Support.
//  Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
//
//---------------------------------------------------------------------------

#define WIN31        // this is a Windows 3.1 application
#define USECOMM      // yes, we need the COMM API
#define STRICT       // be bold!

#include <windows.h>
#include <commdlg.h>
#include <string.h>


#define GWW_NPOutINFO       0
#define ABOUTDLG_USEBITMAP  1

#define ATOM_OutINFO       0x100

// terminal size

//#define MAXROWS         11
#define MAXCOLS         35
#define MAXROWS         22
//#define MAXCOLS         49

#define MAXBLOCK        80

#define MAXLEN_TEMPSTR  81

#define RXQUEUE         4096
#define TXQUEUE         4096

// cursor states

#define CS_HIDE         0x00
#define CS_SHOW         0x01

// Flow control flags

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

// ascii definitions

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

// data structures

typedef struct tagOutINFO
{
   int     idComDev ;
   BYTE    abScreen[ MAXROWS * MAXCOLS ] ;
   BOOL    fLocalEcho, fNewLine, fAutoWrap;
   WORD    wCursorState ;
   HFONT   hOutFont ;
   LOGFONT lfOutFont ;
   DWORD   rgbFGColor ;
   int     xSize, ySize, xScroll, yScroll, xOffset, yOffset,
           nColumn, nRow, xChar, yChar ;
} OutINFO, NEAR *NPOutINFO ;

// macros ( for easier readability )

#define GETHINST( hWnd )  ((HINSTANCE) GetWindowWord( hWnd, GWW_HINSTANCE ))

#define SCREEN( x ) (x -> abScreen)
#define NEWLINE( x ) (x -> fNewLine)
#define AUTOWRAP( x ) (x -> fAutoWrap)
#define CURSORSTATE( x ) (x -> wCursorState)
#define HOutFONT( x ) (x -> hOutFont)
#define LFOutFONT( x ) (x -> lfOutFont)
#define FGCOLOR( x ) (x -> rgbFGColor)
#define XSIZE( x ) (x -> xSize)
#define YSIZE( x ) (x -> ySize)
#define XSCROLL( x ) (x -> xScroll)
#define YSCROLL( x ) (x -> yScroll)
#define XOFFSET( x ) (x -> xOffset)
#define YOFFSET( x ) (x -> yOffset)
#define COLUMN( x ) (x -> nColumn)
#define ROW( x ) (x -> nRow)
#define XCHAR( x ) (x -> xChar)
#define YCHAR( x ) (x -> yChar )

#define SET_PROP( x, y, z )  SetProp( x, MAKEINTATOM( y ), z )
#define GET_PROP( x, y )     GetProp( x, MAKEINTATOM( y ) )
#define REMOVE_PROP( x, y )  RemoveProp( x, MAKEINTATOM( y ) )

// global stuff

char     gszOutClass[] = "OutWndClass" ;
char     gszAppName[] = "Out" ;
HANDLE   ghAccel ;
// function prototypes (private)

LRESULT  CreateOutInfo( HWND ) ;
BOOL  DestroyOutInfo( HWND ) ;
BOOL  ResetOutScreen( HWND, NPOutINFO ) ;
BOOL  KillOutFocus( HWND ) ;
BOOL  PaintOut( HWND ) ;
BOOL  SetOutFocus( HWND ) ;
BOOL  ScrollOutHorz( HWND, WORD, WORD ) ;
BOOL  ScrollOutVert( HWND, WORD, WORD ) ;
BOOL  SizeOut( HWND, WORD, WORD ) ;
BOOL  ProcessOutCharacter( HWND, BYTE ) ;
BOOL  WriteOutBlock( HWND, LPSTR, int ) ;
int  ReadCommBlock( HWND, LPSTR, int ) ;
BOOL  WriteCommByte( HWND, BYTE ) ;
BOOL  MoveOutCursor( HWND ) ;
BOOL  OpenConnection( HWND ) ;
BOOL  SetupConnection( HWND ) ;
BOOL  CloseConnection( HWND ) ;


