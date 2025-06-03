# Microsoft Visual C++ generated build script - Do not modify

PROJ = SCAN
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\SH\GL\SCAN\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = SCANMAX.C   
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Od /D "_DEBUG" /GA /Fd"ce.PDB"
CFLAGS_R_WEXE = /nologo /G3 /W3 /AL /O2 /D "NDEBUG" /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib mmsystem.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib mmsystem.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = 
RESFLAGS = /k 
RUNFLAGS = 
DEFFILE = SCAN.DEF
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
SBRS = SCANMAX.SBR \
		ADCDAC.SBR \
		FCOMM.SBR \
		GRAWIN.SBR \
		MEM.SBR \
		MMATH.SBR \
		PAINT.SBR \
		PORT.SBR \
		WAVEWIN.SBR \
		MPROCESS.SBR


SCANMAX_DEP = c:\sh\gl\commlib\cinc.h \
	c:\sh\gl\commlib\fcomm.h \
	c:\sh\gl\commlib\port.h


ADCDAC_DEP = c:\sh\gl\commlib\cinc.h


FCOMM_DEP = c:\sh\gl\commlib\cinc.h


FCOMM_DEP = c:\sh\gl\commlib\fcomm.h


GRAWIN_DEP = c:\sh\gl\commlib\cinc.h


MEM_DEP = c:\sh\gl\commlib\cinc.h


MMATH_DEP = c:\sh\gl\commlib\cinc.h


PAINT_DEP = c:\sh\gl\commlib\cinc.h


PORT_DEP = c:\sh\gl\commlib\port.h


WAVEWIN_DEP = c:\sh\gl\commlib\cinc.h


MPROCESS_DEP = c:\sh\gl\commlib\cinc.h


all:	$(PROJ).EXE

SCANMAX.OBJ:	SCANMAX.C $(SCANMAX_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c SCANMAX.C

SCAN.RES:	SCAN.RC $(SCAN_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r SCAN.RC

ADCDAC.OBJ:	..\COMMLIB\ADCDAC.C $(ADCDAC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\ADCDAC.C

FCOMM.OBJ:	..\COMMLIB\FCOMM.C $(FCOMM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\FCOMM.C

GRAWIN.OBJ:	..\COMMLIB\GRAWIN.C $(GRAWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\GRAWIN.C

MEM.OBJ:	..\COMMLIB\MEM.C $(MEM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\MEM.C

MMATH.OBJ:	..\COMMLIB\MMATH.C $(MMATH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\MMATH.C

PAINT.OBJ:	..\COMMLIB\PAINT.C $(PAINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\PAINT.C

PORT.OBJ:	..\COMMLIB\PORT.C $(PORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\PORT.C

WAVEWIN.OBJ:	..\COMMLIB\WAVEWIN.C $(WAVEWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\WAVEWIN.C

MPROCESS.OBJ:	..\COMMLIB\MPROCESS.C $(MPROCESS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\MPROCESS.C


$(PROJ).EXE::	SCAN.RES

$(PROJ).EXE::	SCANMAX.OBJ ADCDAC.OBJ FCOMM.OBJ GRAWIN.OBJ MEM.OBJ MMATH.OBJ PAINT.OBJ \
	PORT.OBJ WAVEWIN.OBJ MPROCESS.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
SCANMAX.OBJ +
ADCDAC.OBJ +
FCOMM.OBJ +
GRAWIN.OBJ +
MEM.OBJ +
MMATH.OBJ +
PAINT.OBJ +
PORT.OBJ +
WAVEWIN.OBJ +
MPROCESS.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc_cd\lib\+
c:\msvc_cd\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) SCAN.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	SCAN.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) SCAN.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
