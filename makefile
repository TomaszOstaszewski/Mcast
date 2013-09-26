#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
APPVER=5.01
!include <win32.mak>
#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
OUTDIR_OBJ=$(OUTDIR)\obj
OUTDIR_PCC=$(OUTDIR)\pcc
#----- OUTDIR_OBJ is defined in WIN32.MAK This is the name of the destination directory-------
        
all: $(OUTDIR)\install.msi
	
DXLIB="$(PROGRAMFILES)\Microsoft DirectX SDK (June 2010)\Lib\x86"
# those are the paths for CSCOPE utility for Win32.
# That one does not accept names with spaces in them - thus those paths are presented
# in old 8.3 format.
PSDKINCLUDE="C:\Program Files\Microsoft SDK\Include"
DXINCLUDE="C:\Program Files\Microsoft DirectX SDK (June 2010)\Include"

PSDKINCLUDE_8DOT3="C:\Program Files\MIFD68~1\Include"
DXINCLUDE_8DOT3="C:\Program Files\MI7482~1\Include"

$(OUTDIR) :
    @if not exist "$(OUTDIR_OBJ)/$(NULL)" mkdir $(OUTDIR_OBJ)

$(OUTDIR_PCC) : $(OUTDIR)
    @if not exist "$(OUTDIR_PCC)/$(NULL)" mkdir $(OUTDIR_PCC)

$(OUTDIR_OBJ) : $(OUTDIR)
    @if not exist "$(OUTDIR_OBJ)/$(NULL)" mkdir $(OUTDIR_OBJ)

cscope:
	cscope -bk -I$(PSDKINCLUDE_8DOT3) -I$(DXINCLUDE_8DOT3)

#cscope -bk -I$(DXINCLUDE) $(PSDKINCLUDE)
#.c{$(OUTDIR_OBJ)}.obj::
#    $(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" $<

$(OUTDIR_PCC)\pcc.pch: pcc.c pcc.h $(OUTDIR_PCC) $(OUTDIR_OBJ)
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yc /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_PCC)\pcpp.pch: pcpp.cpp pcc.h $(OUTDIR_PCC) $(OUTDIR_OBJ)
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yc /Fp$(OUTDIR_PCC)\pcpp.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\sender.res: sender.rc sender-res.h sender.exe.manifest play.wav $(OUTDIR_OBJ)
	@$(rc) $(rcflags) $(rcvars) /fo $@ %s

$(OUTDIR_OBJ)\receiver.res: receiver.rc receiver-res.h receiver.exe.manifest $(OUTDIR_OBJ)
	@$(rc) $(rcflags) $(rcvars) /fo $@ %s

$(OUTDIR_OBJ)\version.res: version.rc $(OUTDIR_OBJ)
	@$(rc) $(rcflags) $(rcvars) /fo $@ %s

$(OUTDIR_OBJ)\common-dialogs.res: common-dialogs.rc common-dialogs-res.h $(OUTDIR_OBJ)
	@$(rc) $(rcflags) $(rcvars) /fo $@ %s

$(OUTDIR_OBJ)\abstract-tone.obj: abstract-tone.c abstract-tone.h wave_utils.h $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\about-dialog.obj: about-dialog.c about-dialog.h $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\resample.obj: resample.c
    @$(cc) $(cdebug) $(cvars) $(cflags) /O2 /Oy /I$(DXINCLUDE)  /W3 /WX /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast_utils.obj: mcast_utils.c mcast_utils.h resolve.h $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\resolve.obj: resolve.c resolve.h debug_helpers.h $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\timeofday.obj: timeofday.c timeofday.h $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-state-machine.obj: mcast-sender-state-machine.c mcast-sender-state-machine.h sender-settings.h mcast_setup.h wave_utils.h abstract-tone.h $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-state-machine.obj: mcast-receiver-state-machine.c mcast-receiver-state-machine.h receiver-settings.h play-settings.h mcast_setup.h dsoundplay.h circular-buffer-uint8.h wave_utils.h $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast_setup.obj: mcast_setup.c mcast_setup.h mcast_utils.h resolve.h mcast-settings.h $(OUTDIR_OBJ)
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\debug_helpers.obj: debug_helpers.c pcc.h debug_helpers.h pcc.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE) /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-dlg.obj: mcast-receiver-dlg.c pcc.h dsoundplay.h mcast_setup.h mcast-settings.h mcast-receiver-state-machine.h circular-buffer-uint8.h receiver-settings.h play-settings.h wave_utils.h receiver-res.h $(OUTDIR_OBJ)
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-dlg.obj: mcast-sender-dlg.c sender-settings-dlg.h mcast-sender-state-machine.h pcc.h sender-settings.h sender-res.h abstract-tone.h wave_utils.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-settings.obj: mcast-settings.c mcast-settings.h pcc.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-settings-dlg.obj: mcast-settings-dlg.c mcast-settings-dlg.h pcc.h common-dialogs-res.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\sender-settings-dlg.obj: sender-settings-dlg.c pcc.h sender-settings-dlg.h sender-settings.h sender-res.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\sender-settings.obj: sender-settings.c pcc.h sender-settings.h wave_utils.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\platform-sockets.obj: platform-sockets.c platform-sockets.h pcc.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\receiver-settings.obj: receiver-settings.c pcc.h receiver-settings.h play-settings.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\receiver-settings-dlg.obj: receiver-settings-dlg.c pcc.h receiver-settings.h play-settings.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\wave_utils.obj: wave_utils.c wave_utils.h pcc.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\dsoundplay.obj: dsoundplay.cpp dsoundplay.h pcc.h wave_utils.h circular-buffer-uint8.h input-buffer.h receiver-settings.h play-settings.h perf-counter-itf.h  $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcpp.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcpp.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\message-loop.obj: message-loop.c message-loop.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\circular-buffer-uint8.obj: circular-buffer-uint8.c circular-buffer-uint8.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @if exist "$(OUTDIR_OBJ)\$(@B).S" del /Q /F "$(OUTDIR_OBJ)\$(@B).S"
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\input-buffer.obj: input-buffer.c input-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\play-settings.obj: play-settings.c play-settings.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\perf-counter-itf.obj: perf-counter-itf.c perf-counter-itf.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-input-buffer.obj: ut-input-buffer.c input-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-debug-helpers.obj: ut-debug-helpers.c debug_helpers.c $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-circular-buffer-uint8.obj: ut-circular-buffer-uint8.c circular-buffer-uint8.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ex-perf-counter.obj: ex-perf-counter.c perf-counter-itf.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @if exist "$(OUTDIR_OBJ)\$(@B).S" del /Q /F "$(OUTDIR_OBJ)\$(@B).S"
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\dialog-utils.obj: dialog-utils.c dialog-utils.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @if exist "$(OUTDIR_OBJ)\$(@B).S" del /Q /F "$(OUTDIR_OBJ)\$(@B).S"
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-abstract-tone.obj: ut-abstract-tone.c abstract-tone.h sender-res.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @$(cc) $(cdebug) $(cvars) $(cflags) /I$(DXINCLUDE)  /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

# Tests
$(OUTDIR)\ut-abstract-tone.exe: $(OUTDIR_OBJ)\ut-abstract-tone.obj $(OUTDIR_OBJ)\abstract-tone.obj $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\wave_utils.obj $(OUTDIR_OBJ)\sender.res 
	@$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\ut-debug-helpers.exe: $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\ut-debug-helpers.obj 
	@$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib
	
$(OUTDIR)\ut-circular-buffer-uint8.exe: $(OUTDIR_OBJ)\circular-buffer-uint8.obj $(OUTDIR_OBJ)\ut-circular-buffer-uint8.obj $(OUTDIR_OBJ)\timeofday.obj
	@$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib
	
$(OUTDIR)\ut-input-buffer.exe: $(OUTDIR_OBJ)\input-buffer.obj $(OUTDIR_OBJ)\ut-input-buffer.obj 
	@$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\ex-perf-counter.exe: $(OUTDIR_OBJ)\ex-perf-counter.obj $(OUTDIR_OBJ)\perf-counter-itf.obj
	@$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\resample.exe: $(OUTDIR_OBJ)\resample.obj
	@$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb-out:$@ $** $(guilibs)

tests: \
 $(OUTDIR)\ut-abstract-tone.exe \
 $(OUTDIR)\ut-debug-helpers.exe \
 $(OUTDIR)\ut-circular-buffer-uint8.exe \
 $(OUTDIR)\ut-input-buffer.exe \
 $(OUTDIR)\ex-perf-counter.exe
	
$(OUTDIR)\debughelpers.lib:\
 $(OUTDIR_OBJ)\version.res\
 $(OUTDIR_OBJ)\debug_helpers.obj
	@$(link) /DEF:$(@B).def /dll $(ldebug) $(guiflags) /NOLOGO /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$(OUTDIR)\$(@B).dll $** $(guilibs) dsound.lib winmm.lib dxguid.lib 

$(OUTDIR)\dsoundplay.lib: $(OUTDIR)\debughelpers.lib

$(OUTDIR)\dsoundplay.lib:\
 $(OUTDIR_OBJ)\version.res\
 $(OUTDIR_OBJ)\dsoundplay.obj\
 $(OUTDIR_OBJ)\input-buffer.obj\
 $(OUTDIR_OBJ)\circular-buffer-uint8.obj\
 $(OUTDIR_OBJ)\receiver-settings.obj\
 $(OUTDIR_OBJ)\mcast-settings.obj\
 $(OUTDIR_OBJ)\play-settings.obj\
 $(OUTDIR_OBJ)\perf-counter-itf.obj\
 $(OUTDIR_OBJ)\wave_utils.obj
	@$(link) /DEF:dsoundplay.def /dll $(ldebug) $(guiflags) /NOLOGO /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$(OUTDIR)\$(@B).dll $** $(guilibs) dsound.lib winmm.lib dxguid.lib 

$(OUTDIR)\mcast.lib: \
 $(OUTDIR)\debughelpers.lib \
 $(OUTDIR_OBJ)\version.res\
 $(OUTDIR_OBJ)\mcast_utils.obj\
 $(OUTDIR_OBJ)\platform-sockets.obj\
 $(OUTDIR_OBJ)\resolve.obj
	@$(link) /DEF:$(@B).def /dll $(ldebug) $(guiflags) /NOLOGO /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$(OUTDIR)\$(@B).dll $** $(guilibs) dsound.lib winmm.lib dxguid.lib 
 
$(OUTDIR)\receiver.exe: $(OUTDIR)\dsoundplay.lib $(OUTDIR)\debughelpers.lib $(OUTDIR)\mcast.lib

$(OUTDIR)\receiver.exe: \
 $(OUTDIR_OBJ)\version.res\
 $(OUTDIR_OBJ)\receiver.res\
 $(OUTDIR_OBJ)\common-dialogs.res\
 $(OUTDIR_OBJ)\timeofday.obj\
 $(OUTDIR_OBJ)\mcast_setup.obj\
 $(OUTDIR_OBJ)\message-loop.obj\
 $(OUTDIR_OBJ)\mcast-receiver-state-machine.obj\
 $(OUTDIR_OBJ)\receiver-settings.obj\
 $(OUTDIR_OBJ)\mcast-settings.obj\
 $(OUTDIR_OBJ)\play-settings.obj\
 $(OUTDIR_OBJ)\mcast-receiver-dlg.obj\
 $(OUTDIR_OBJ)\mcast-settings-dlg.obj\
 $(OUTDIR_OBJ)\receiver-settings-dlg.obj\
 $(OUTDIR_OBJ)\about-dialog.obj\
 $(OUTDIR_OBJ)\perf-counter-itf.obj\
 $(OUTDIR_OBJ)\dialog-utils.obj
	@$(link) $(ldebug) $(guiflags) /NOLOGO /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$@ $** $(guilibs) ComCtl32.lib ole32.lib Version.lib

$(OUTDIR)\sender.exe: $(OUTDIR)\dsoundplay.lib $(OUTDIR)\debughelpers.lib $(OUTDIR)\mcast.lib

$(OUTDIR)\sender.exe: \
 $(OUTDIR_OBJ)\version.res\
 $(OUTDIR_OBJ)\sender.res\
 $(OUTDIR_OBJ)\common-dialogs.res\
 $(OUTDIR_OBJ)\timeofday.obj\
 $(OUTDIR_OBJ)\mcast_setup.obj\
 $(OUTDIR_OBJ)\message-loop.obj\
 $(OUTDIR_OBJ)\mcast-sender-state-machine.obj\
 $(OUTDIR_OBJ)\sender-settings.obj\
 $(OUTDIR_OBJ)\dialog-utils.obj\
 $(OUTDIR_OBJ)\mcast-settings.obj\
 $(OUTDIR_OBJ)\mcast-sender-dlg.obj\
 $(OUTDIR_OBJ)\sender-settings-dlg.obj\
 $(OUTDIR_OBJ)\mcast-settings-dlg.obj\
 $(OUTDIR_OBJ)\about-dialog.obj\
 $(OUTDIR_OBJ)\perf-counter-itf.obj\
 $(OUTDIR_OBJ)\abstract-tone.obj
	@$(link) $(ldebug) $(guiflags) /NOLOGO /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$@ $** $(guilibs) ComCtl32.lib winmm.lib dxguid.lib ole32.lib Version.lib

$(OUTDIR)\install.wix: install.wix $(OUTDIR)
	@copy /Y install.wix $(OUTDIR)\install.wix
$(OUTDIR)\install.wixobj: install.wix
	@candle -nologo -doutputdir=$(OUTDIR) -o $(OUTDIR)\\install.wixobj $**

app:  \
$(OUTDIR)\sender.exe \
$(OUTDIR)\receiver.exe

$(OUTDIR)\install.msi: \
 $(OUTDIR)\install.wix \
 $(OUTDIR)\install.wixobj \
 $(OUTDIR)\sender.exe \
 $(OUTDIR)\receiver.exe \
 $(OUTDIR)\dsoundplay.dll \
 $(OUTDIR)\mcast.dll \
 $(OUTDIR)\debughelpers.dll
	@light -nologo -ext WixUIExtension -o $(OUTDIR)\install.msi $(OUTDIR)\install.wixobj

install: $(OUTDIR)\install.msi

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
	$(CLEANUP)
