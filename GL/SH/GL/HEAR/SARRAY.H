#define mqueue 200 // ������ �������
//#define motstup 5  // �����, �� �������� ���� ������� ������

extern double queuelevel;

//static double f[mqueue]; // ����������� ��������
  // ������� - ��� ������������ ���������� / ��������
//static int p[mqueue], // ��������� � f.
//          bp[mqueue]; // �������� ������.
//static tp = 0; // ���������� ���������� ��������� � p[].

VOID initqueue( double f0 );
VOID addqueue( double f0 );
BOOL testqueue( void );

