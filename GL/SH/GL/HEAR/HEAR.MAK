# Microsoft Visual C++ generated build script - Do not modify

PROJ = HEAR
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\SH\GL\HEAR\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = ADCDAC.C    
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
DEFFILE = HEAR.DEF
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
SBRS = ADCDAC.SBR \
		FCOMM.SBR \
		GRAWIN.SBR \
		MEM.SBR \
		MMATH.SBR \
		PAINT.SBR \
		WAVEWIN.SBR \
		MPROCESS.SBR \
		HEAR.SBR \
		OUTWIN.SBR \
		SARRAY.SBR


ADCDAC_DEP = c:\sh\gl\commlib\cinc.h


FCOMM_DEP = c:\sh\gl\commlib\cinc.h \
	c:\sh\gl\commlib\fcomm.h


GRAWIN_DEP = c:\sh\gl\commlib\cinc.h


MEM_DEP = c:\sh\gl\commlib\cinc.h


MMATH_DEP = c:\sh\gl\commlib\cinc.h


PAINT_DEP = c:\sh\gl\commlib\cinc.h


WAVEWIN_DEP = c:\sh\gl\commlib\cinc.h


MPROCESS_DEP = c:\sh\gl\commlib\cinc.h


HEAR_DEP = c:\sh\gl\commlib\cinc.h \
	c:\sh\gl\commlib\fcomm.h \
	c:\sh\gl\hear\sarray.h


OUTWIN_DEP = c:\sh\gl\commlib\outwin.h


SARRAY_DEP = c:\sh\gl\commlib\cinc.h


all:	$(PROJ).EXE

ADCDAC.OBJ:	..\COMMLIB\ADCDAC.C $(ADCDAC_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c ..\COMMLIB\ADCDAC.C

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

WAVEWIN.OBJ:	..\COMMLIB\WAVEWIN.C $(WAVEWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\WAVEWIN.C

MPROCESS.OBJ:	..\COMMLIB\MPROCESS.C $(MPROCESS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\MPROCESS.C

HEAR.OBJ:	HEAR.C $(HEAR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEAR.C

HEAR.RES:	HEAR.RC $(HEAR_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r HEAR.RC

OUTWIN.OBJ:	..\COMMLIB\OUTWIN.C $(OUTWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\COMMLIB\OUTWIN.C

SARRAY.OBJ:	SARRAY.C $(SARRAY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SARRAY.C


$(PROJ).EXE::	HEAR.RES

$(PROJ).EXE::	ADCDAC.OBJ FCOMM.OBJ GRAWIN.OBJ MEM.OBJ MMATH.OBJ PAINT.OBJ WAVEWIN.OBJ \
	MPROCESS.OBJ HEAR.OBJ OUTWIN.OBJ SARRAY.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
ADCDAC.OBJ +
FCOMM.OBJ +
GRAWIN.OBJ +
MEM.OBJ +
MMATH.OBJ +
PAINT.OBJ +
WAVEWIN.OBJ +
MPROCESS.OBJ +
HEAR.OBJ +
OUTWIN.OBJ +
SARRAY.OBJ +
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
	$(RC) $(RESFLAGS) HEAR.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	HEAR.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) HEAR.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
