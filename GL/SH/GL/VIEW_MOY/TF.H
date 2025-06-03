#include <windows.h>
#include <mmsystem.h>
   
LRESULT CALLBACK _export 
     X_WndProc(HWND hwnd,UINT msg,
                  WPARAM wParam,LPARAM lParam);

static double FAR *x_wndrezvect;
static int x_wndrezlen;

class TF
{ private:
   HWND hwnd;
   char Name[20],Title[20];
   
   int errno;
   double FAR *rezvect;
   int size;
  public:
  //constructor
  TF(LPSTR,LPSTR,int);
  //destructor
  ~TF();
  //inicializazia okna
  BOOL InitInstance(HINSTANCE,HWND,int,int,int,int);
  //otobrajenie
  VOID Show(int nCmdShow);
  //on WM_PAINT
  VOID Update(VOID);
  //cbos rezultatov
  VOID clear(VOID);
  //nakoplenie rezultatov
  VOID process(double FAR *sr,double FAR *sl);
};