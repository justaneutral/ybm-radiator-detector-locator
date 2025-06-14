#include "cinc.h"

VOID initproc( sprocess FAR *p ) {
  p->state = 0;
  p->stackptr = 0;
}


VOID callstateproc( sprocess FAR *p, short gotoStateNom, short retState ) {
// ��������� ��������� �������� � ����������� ��������� ��� 
// �������� � ���� ��������� �������� (�������� ������ ���������).

  if (p->stackptr >= mstatestack) {
    message(NULL,"stack error");
    error = TRUE;
    p->stackptr--;
    return;
  }
  
  p->statestack[p->stackptr] = retState;
  p->state = gotoStateNom;  
  p->stackptr++;
}

VOID retstateproc( sprocess FAR *p ) {
// ��������� ��������� �������� �� �������� �� �����
// (������� �� ���������).

  p->stackptr--;
  if ( p->stackptr < 0 ) {
    message(NULL,"empty stack error");
    error = TRUE;
    p->stackptr = 0;
    return;
  }
  p->state = p->statestack[p->stackptr];  
}

// **************************************************************
