# Microsoft Visual C++ generated build script - Do not modify

PROJ = GL
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\GADV\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = MIDI.C      
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Od /D "_DEBUG" /GA /Fd"GL.PDB"
CFLAGS_R_WEXE = /nologo /G3 /W3 /AL /O2 /D "NDEBUG" /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib mmsystem.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib mmsystem.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = 
RESFLAGS = /k 
RUNFLAGS = 
DEFFILE = GL.DEF
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
SBRS = PAINT.SBR \
		ADCDAC.SBR \
		GANTELIA.SBR \
		MEM.SBR \
		MMATH.SBR \
		START.SBR \
		SCAN.SBR \
		SCANMAX.SBR \
		FCOMM.SBR \
		WAVEWIN.SBR \
		GRAWIN.SBR \
		DIRWIN.SBR \
		SCALEWIN.SBR \
		CHECK.SBR \
		PORT.SBR \
		MIDI.SBR \
		POWERWIN.SBR \
		RSOLV.SBR


PAINT_DEP = c:\gadv\cinc.h


ADCDAC_DEP = c:\gadv\cinc.h


GANTELIA_DEP = c:\gadv\cinc.h


GANTELIA_DEP = c:\gadv\port.h


MEM_DEP = c:\gadv\cinc.h


MMATH_DEP = c:\gadv\cinc.h


START_DEP = c:\gadv\cinc.h


SCAN_DEP = c:\gadv\cinc.h \
	c:\gadv\fcomm.h \
	c:\gadv\scan.h


SCANMAX_DEP = c:\gadv\cinc.h \
	c:\gadv\fcomm.h \
	c:\gadv\scan.h


FCOMM_DEP = c:\gadv\cinc.h \
	c:\gadv\fcomm.h


WAVEWIN_DEP = c:\gadv\cinc.h


GRAWIN_DEP = c:\gadv\cinc.h


DIRWIN_DEP = c:\gadv\cinc.h


SCALEWIN_DEP = c:\gadv\cinc.h \
	c:\gadv\scan.h


CHECK_DEP = c:\gadv\cinc.h \
	c:\gadv\fcomm.h \
	c:\gadv\scan.h


PORT_DEP = c:\gadv\port.h


MIDI_DEP = c:\gadv\cinc.h


POWERWIN_DEP = c:\gadv\cinc.h \
	c:\gadv\scan.h


RSOLV_DEP = c:\gadv\cinc.h


all:	$(PROJ).EXE

GL.RES:	GL.RC $(GL_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r GL.RC

PAINT.OBJ:	PAINT.C $(PAINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PAINT.C

ADCDAC.OBJ:	ADCDAC.C $(ADCDAC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ADCDAC.C

GANTELIA.OBJ:	GANTELIA.C $(GANTELIA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GANTELIA.C

MEM.OBJ:	MEM.C $(MEM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MEM.C

MMATH.OBJ:	MMATH.C $(MMATH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MMATH.C

START.OBJ:	START.C $(START_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c START.C

SCAN.OBJ:	SCAN.C $(SCAN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SCAN.C

SCANMAX.OBJ:	SCANMAX.C $(SCANMAX_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SCANMAX.C

FCOMM.OBJ:	FCOMM.C $(FCOMM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FCOMM.C

WAVEWIN.OBJ:	WAVEWIN.C $(WAVEWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c WAVEWIN.C

GRAWIN.OBJ:	GRAWIN.C $(GRAWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAWIN.C

DIRWIN.OBJ:	DIRWIN.C $(DIRWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DIRWIN.C

SCALEWIN.OBJ:	SCALEWIN.C $(SCALEWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SCALEWIN.C

CHECK.OBJ:	CHECK.C $(CHECK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CHECK.C

PORT.OBJ:	PORT.C $(PORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PORT.C

MIDI.OBJ:	MIDI.C $(MIDI_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c MIDI.C

POWERWIN.OBJ:	POWERWIN.C $(POWERWIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c POWERWIN.C

RSOLV.OBJ:	RSOLV.C $(RSOLV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c RSOLV.C


$(PROJ).EXE::	GL.RES

$(PROJ).EXE::	PAINT.OBJ ADCDAC.OBJ GANTELIA.OBJ MEM.OBJ MMATH.OBJ START.OBJ SCAN.OBJ \
	SCANMAX.OBJ FCOMM.OBJ WAVEWIN.OBJ GRAWIN.OBJ DIRWIN.OBJ SCALEWIN.OBJ CHECK.OBJ PORT.OBJ \
	MIDI.OBJ POWERWIN.OBJ RSOLV.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
PAINT.OBJ +
ADCDAC.OBJ +
GANTELIA.OBJ +
MEM.OBJ +
MMATH.OBJ +
START.OBJ +
SCAN.OBJ +
SCANMAX.OBJ +
FCOMM.OBJ +
WAVEWIN.OBJ +
GRAWIN.OBJ +
DIRWIN.OBJ +
SCALEWIN.OBJ +
CHECK.OBJ +
PORT.OBJ +
MIDI.OBJ +
POWERWIN.OBJ +
RSOLV.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvcinst.all\lib\+
c:\msvcinst.all\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) GL.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	GL.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) GL.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
