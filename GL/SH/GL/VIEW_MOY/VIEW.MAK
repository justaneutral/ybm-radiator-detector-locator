# Microsoft Visual C++ generated build script - Do not modify

PROJ = VIEW
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\SH\GL\VIEW\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = TF.CPP      
RC = rc
CFLAGS_D_WEXE = /nologo /G3 /W3 /Zi /AL /Od /D "_DEBUG" /GA /Fd"ce.PDB"
CFLAGS_R_WEXE = /nologo /G3 /W3 /AL /O2 /D "NDEBUG" /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib mmsystem.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib mmsystem.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = 
RESFLAGS = /k 
RUNFLAGS = 
DEFFILE = VIEW.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = TF.SBR \
		VIEW.SBR \
		ADCDAC.SBR \
		FCOMM.SBR \
		PORT.SBR \
		GRAWIN.SBR \
		MEM.SBR \
		MMATH.SBR \
		MPROCESS.SBR \
		PAINT.SBR \
		WAVEWIN.SBR


TF_DEP = c:\sh\gl\view\stdafx.h


TF_DEP = c:\sh\gl\view\tf.h


VIEW_DEP = c:\sh\gl\commlib\cinc.h \
	c:\sh\gl\commlib\fcomm.h \
	c:\sh\gl\commlib\scan.h \
	c:\sh\gl\commlib\port.h \
	c:\sh\gl\view\tf.h


ADCDAC_DEP = c:\sh\gl\commlib\cinc.h


FCOMM_DEP = c:\sh\gl\commlib\cinc.h \
	c:\sh\gl\commlib\fcomm.h


PORT_DEP = c:\sh\gl\commlib\port.h


GRAWIN_DEP = c:\sh\gl\commlib\cinc.h


MEM_DEP = c:\sh\gl\commlib\cinc.h


MMATH_DEP = c:\sh\gl\commlib\cinc.h


MPROCESS_DEP = c:\sh\gl\commlib\cinc.h


PAINT_DEP = c:\sh\gl\commlib\cinc.h


WAVEWIN_DEP = c:\sh\gl\commlib\cinc.h


all:	$(PROJ).EXE

VIEW.RES:	VIEW.RC $(VIEW_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r VIEW.RC

TF.OBJ:	TF.CPP $(TF_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c TF.CPP

VIEW.OBJ:	VIEW.CPP $(VIEW_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c VIEW.CPP

ADCDAC.OBJ:	..\COMMLIB\ADCDAC.CPP $(ADCDAC_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\ADCDAC.CPP

FCOMM.OBJ:	..\COMMLIB\FCOMM.CPP $(FCOMM_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\FCOMM.CPP

PORT.OBJ:	..\COMMLIB\PORT.CPP $(PORT_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\PORT.CPP

GRAWIN.OBJ:	..\COMMLIB\GRAWIN.CPP $(GRAWIN_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\GRAWIN.CPP

MEM.OBJ:	..\COMMLIB\MEM.CPP $(MEM_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\MEM.CPP

MMATH.OBJ:	..\COMMLIB\MMATH.CPP $(MMATH_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\MMATH.CPP

MPROCESS.OBJ:	..\COMMLIB\MPROCESS.CPP $(MPROCESS_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\MPROCESS.CPP

PAINT.OBJ:	..\COMMLIB\PAINT.CPP $(PAINT_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\PAINT.CPP

WAVEWIN.OBJ:	..\COMMLIB\WAVEWIN.CPP $(WAVEWIN_DEP)
	$(CPP) $(CFLAGS) $(CPPUSEPCHFLAG) /c ..\COMMLIB\WAVEWIN.CPP


$(PROJ).EXE::	VIEW.RES

$(PROJ).EXE::	TF.OBJ VIEW.OBJ ADCDAC.OBJ FCOMM.OBJ PORT.OBJ GRAWIN.OBJ MEM.OBJ MMATH.OBJ \
	MPROCESS.OBJ PAINT.OBJ WAVEWIN.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
TF.OBJ +
VIEW.OBJ +
ADCDAC.OBJ +
FCOMM.OBJ +
PORT.OBJ +
GRAWIN.OBJ +
MEM.OBJ +
MMATH.OBJ +
MPROCESS.OBJ +
PAINT.OBJ +
WAVEWIN.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvcinst.all\lib\+
c:\msvcinst.all\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) VIEW.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	VIEW.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) VIEW.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
