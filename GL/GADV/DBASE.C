#include "windows.h"

BOOL StationError = FALSE;

#define mSpec 64
#define mStation 2000

typedef double Spec[ mSpec ];

typedef struct { char v; long f; } FreqValue;

typedef struct 
  { 
    double s; // ���������
    HANDLE h; // ���������� � �������
    long fLow, fHigh, f; // �������� ������
    char v;   // ������� ������� ( ��� ��������� ).
  } StationHandle; 

typedef struct {
  long nCh;     // ����� ��������
  Spec SpecIn,  // �������� / MIDI
       SpecOut; // ��������
} StationData;

StationHandle htab[mStation];

BOOL SaveStation(HWND hWnd, LPCSTR FileName);

BOOL LoadStation(HWND hWnd, LPCSTR FileName);

BOOL EmptyStation(HWND hWnd);

