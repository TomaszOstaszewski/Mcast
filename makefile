#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include <win32.mak>
#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
OUTDIR_OBJ=$(OUTDIR)\obj
OUTDIR_PCC=$(OUTDIR)\pcc
#----- OUTDIR_OBJ is defined in WIN32.MAK This is the name of the destination directory-------
        
all: $(OUTDIR)\ex-perf-counter.exe $(OUTDIR)\ut-fifo-circular-buffer.exe $(OUTDIR)\ut-input-buffer.exe $(OUTDIR)\sender.exe $(OUTDIR)\receiver.exe
	@echo $(OUTDIR)\ut-fifo-circular-buffer.exe
	@echo $(OUTDIR)\ut-input-buffer.exe
	
DXLIB="$(PROGRAMFILES)\Microsoft DirectX SDK (June 2010)\Lib\x86"
# those are the paths for CSCOPE utility for Win32.
# That one does not accept names with spaces in them - thus those paths are presented
# in old 8.3 format.
PSDKINCLUDE=C:\PROGRA~1\MI9547~1\Include
DXINCLUDE=C:\PROGRA~1\MI7482~1\Include

$(OUTDIR):
    @if not exist "$(OUTDIR_OBJ)/$(NULL)" mkdir $(OUTDIR_OBJ)

$(OUTDIR_PCC) : $(OUTDIR)
    @if not exist "$(OUTDIR_PCC)/$(NULL)" mkdir $(OUTDIR_PCC)

$(OUTDIR_OBJ) : $(OUTDIR)
    @if not exist "$(OUTDIR_OBJ)/$(NULL)" mkdir $(OUTDIR_OBJ)!include "outdir.mk"

cscope:
	@cscope -bk -I$(PSDKINCLUDE) -I$(DXINCLUDE)

#cscope -bk -I$(DXINCLUDE) $(PSDKINCLUDE)
#.c{$(OUTDIR_OBJ)}.obj::
#    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" $<

$(OUTDIR_PCC)\pcc.pch: pcc.c pcc.h $(OUTDIR_PCC) $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yc /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_PCC)\pcpp.pch: pcpp.cpp pcc.h $(OUTDIR_PCC) $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yc /Fp$(OUTDIR_PCC)\pcpp.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\sender.res: sender.rc sender-res.h sender.exe.manifest play.wav $(OUTDIR_OBJ)
	$(rc) $(rcflags) $(rcvars) /fo $@ %s

$(OUTDIR_OBJ)\receiver.res: receiver.rc receiver-res.h receiver.exe.manifest $(OUTDIR_OBJ)
	$(rc) $(rcflags) $(rcvars) /fo $@ %s

$(OUTDIR_OBJ)\common-dialogs.res: common-dialogs.rc common-dialogs-res.h $(OUTDIR_OBJ)
	$(rc) $(rcflags) $(rcvars) /fo $@ %s

$(OUTDIR_OBJ)\about-dialog.obj: about-dialog.c about-dialog.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast_utils.obj: mcast_utils.c mcast_utils.h resolve.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\resolve.obj: resolve.c resolve.h debug_helpers.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\timeofday.obj: timeofday.c timeofday.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-state-machine.obj: mcast-sender-state-machine.c mcast-sender-state-machine.h sender-settings.h mcast_setup.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-state-machine.obj: mcast-receiver-state-machine.c mcast-receiver-state-machine.h receiver-settings.h play-settings.h mcast_setup.h dsoundplay.h fifo-circular-buffer.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast_setup.obj: mcast_setup.c mcast_setup.h mcast_utils.h resolve.h mcast-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\debug_helpers.obj: debug_helpers.c pcc.h debug_helpers.h  $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-dlg.obj: mcast-receiver-dlg.c pcc.h dsoundplay.h mcast_setup.h mcast-settings.h mcast-receiver-state-machine.h fifo-circular-buffer.h receiver-settings.h play-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-dlg.obj: mcast-sender-dlg.c sender-settings-dlg.h mcast-sender-state-machine.h pcc.h sender-settings.h sender-res.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-settings.obj: mcast-settings.c mcast-settings.h pcc.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-settings-dlg.obj: mcast-settings-dlg.c mcast-settings-dlg.h pcc.h common-dialogs-res.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\sender-settings-dlg.obj: sender-settings-dlg.c pcc.h sender-settings-dlg.h sender-settings.h sender-res.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\sender-settings.obj: sender-settings.c pcc.h sender-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\receiver-settings.obj: receiver-settings.c pcc.h receiver-settings.h play-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\receiver-settings-dlg.obj: receiver-settings-dlg.c pcc.h receiver-settings.h play-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\wave_utils.obj: wave_utils.c pcc.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\dsoundplay.obj: dsoundplay.cpp fifo-circular-buffer.h input-buffer.h receiver-settings.h play-settings.h perf-counter-itf.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcpp.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcpp.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\message-loop.obj: message-loop.c message-loop.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\fifo-circular-buffer.obj: fifo-circular-buffer.c fifo-circular-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @if exist "$(OUTDIR_OBJ)\$(@B).S" del /Q /F "$(OUTDIR_OBJ)\$(@B).S"
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\input-buffer.obj: input-buffer.c input-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\play-settings.obj: play-settings.c play-settings.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\perf-counter-itf.obj: perf-counter-itf.c perf-counter-itf.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-input-buffer.obj: ut-input-buffer.c input-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-fifo-circular-buffer.obj: ut-fifo-circular-buffer.c fifo-circular-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ex-perf-counter.obj: ex-perf-counter.c perf-counter-itf.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @if exist "$(OUTDIR_OBJ)\$(@B).S" del /Q /F "$(OUTDIR_OBJ)\$(@B).S"
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\dialog-utils.obj: dialog-utils.c dialog-utils.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    @if exist "$(OUTDIR_OBJ)\$(@B).S" del /Q /F "$(OUTDIR_OBJ)\$(@B).S"
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /FAcs /Fa"$(OUTDIR_OBJ)\\"$(@B).S /Fd"$(OUTDIR_OBJ)\\" %s

# Tests
$(OUTDIR)\ut-fifo-circular-buffer.exe: $(OUTDIR_OBJ)\fifo-circular-buffer.obj $(OUTDIR_OBJ)\ut-fifo-circular-buffer.obj $(OUTDIR_OBJ)\timeofday.obj
	@ECHO $@
	$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib
	
$(OUTDIR)\ut-input-buffer.exe: $(OUTDIR_OBJ)\input-buffer.obj $(OUTDIR_OBJ)\ut-input-buffer.obj 
	@ECHO $@
	$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\ex-perf-counter.exe: $(OUTDIR_OBJ)\ex-perf-counter.obj $(OUTDIR_OBJ)\perf-counter-itf.obj
	@ECHO $@
	$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map /PDB:$(OUTDIR_OBJ)\$(@B).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

# Applications
$(OUTDIR)\receiver.exe: $(OUTDIR_OBJ)\receiver.res $(OUTDIR_OBJ)\common-dialogs.res $(OUTDIR_OBJ)\mcast-receiver-dlg.obj $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\input-buffer.obj $(OUTDIR_OBJ)\fifo-circular-buffer.obj $(OUTDIR_OBJ)\dsoundplay.obj $(OUTDIR_OBJ)\mcast_setup.obj $(OUTDIR_OBJ)\mcast_utils.obj $(OUTDIR_OBJ)\timeofday.obj $(OUTDIR_OBJ)\resolve.obj $(OUTDIR_OBJ)\message-loop.obj  $(OUTDIR_OBJ)\mcast-receiver-state-machine.obj $(OUTDIR_OBJ)\receiver-settings.obj $(OUTDIR_OBJ)\receiver-settings-dlg.obj $(OUTDIR_OBJ)\mcast-settings.obj $(OUTDIR_OBJ)\mcast-settings-dlg.obj $(OUTDIR_OBJ)\play-settings.obj $(OUTDIR_OBJ)\about-dialog.obj $(OUTDIR_OBJ)\perf-counter-itf.obj $(OUTDIR_OBJ)\dialog-utils.obj
	@ECHO $@
	$(link) $(ldebug) $(guiflags) /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib Version.lib

$(OUTDIR)\sender.exe: $(OUTDIR_OBJ)\sender.res $(OUTDIR_OBJ)\common-dialogs.res $(OUTDIR_OBJ)\timeofday.obj $(OUTDIR_OBJ)\mcast_utils.obj $(OUTDIR_OBJ)\resolve.obj $(OUTDIR_OBJ)\mcast_setup.obj $(OUTDIR_OBJ)\mcast-sender-dlg.obj $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\message-loop.obj $(OUTDIR_OBJ)\wave_utils.obj $(OUTDIR_OBJ)\mcast-sender-state-machine.obj $(OUTDIR_OBJ)\sender-settings-dlg.obj $(OUTDIR_OBJ)\sender-settings.obj $(OUTDIR_OBJ)\mcast-settings.obj  $(OUTDIR_OBJ)\mcast-settings-dlg.obj $(OUTDIR_OBJ)\about-dialog.obj $(OUTDIR_OBJ)\perf-counter-itf.obj $(OUTDIR_OBJ)\dialog-utils.obj
	@ECHO $@
	$(link) $(ldebug) $(guiflags) /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib Version.lib

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
	$(CLEANUP)
