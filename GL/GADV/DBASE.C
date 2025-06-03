#include "windows.h"

BOOL StationError = FALSE;

#define mSpec 64
#define mStation 2000

typedef double Spec[ mSpec ];

typedef struct { char v; long f; } FreqValue;

typedef struct 
  { 
    double s; // приоритет
    HANDLE h; // информация о станции
    long fLow, fHigh, f; // диапазон частот
    char v;   // уровень сигнала ( для рисования ).
  } StationHandle; 

typedef struct {
  long nCh;     // число проверок
  Spec SpecIn,  // микрофон / MIDI
       SpecOut; // приемник
} StationData;

StationHandle htab[mStation];

BOOL SaveStation(HWND hWnd, LPCSTR FileName);

BOOL LoadStation(HWND hWnd, LPCSTR FileName);

BOOL EmptyStation(HWND hWnd);

