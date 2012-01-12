#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include "obj.mk"
#----- OUTDIR_OBJ is defined in WIN32.MAK This is the name of the destination directory-------
        
all: $(OUTDIR)\ut-fifo-circular-buffer.exe $(OUTDIR)\ut-input-buffer.exe $(OUTDIR)\sender.exe $(OUTDIR)\receiver.exe
	@echo $(OUTDIR)\ut-fifo-circular-buffer.exe
	@echo $(OUTDIR)\ut-input-buffer.exe
	
DXLIB="C:\Program Files\Microsoft DirectX SDK (June 2010)\Lib\x86"

!include "outdir.mk"

#
# Build rule for resource file
#cdebug=$(cdebug) -DUSE_OWN_STDINT_H

#.c{$(OUTDIR_OBJ)}.obj::
#    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" $<

$(OUTDIR_PCC)\pcc.pch: pcc.c pcc.h $(OUTDIR_PCC) $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yc /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_PCC)\pcpp.pch: pcpp.cpp pcc.h $(OUTDIR_PCC) $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yc /Fp$(OUTDIR_PCC)\pcpp.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcastui.res: mcastui.rc resource.h $(OUTDIR_OBJ)
	$(rc) $(rcflags) $(rcvars) /fo $(OUTDIR_OBJ)\mcastui.res %s

$(OUTDIR_OBJ)\mcast_utils.obj: mcast_utils.c mcast_utils.h resolve.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\resolve.obj: resolve.c resolve.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\timeofday.obj: timeofday.c timeofday.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\winsock_adapter.obj: winsock_adapter.c timeofday.h winsock_adapter.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-state-machine.obj: mcast-sender-state-machine.c mcast-sender-state-machine.h mcast-sender-settings.h mcast_setup.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-state-machine.obj: mcast-receiver-state-machine.c mcast-receiver-state-machine.h mcast_setup.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast_setup.obj: mcast_setup.c winsock_adapter.h mcast_setup.h mcast_utils.h winsock_adapter.h resolve.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\debug_helpers.obj: debug_helpers.c pcc.h debug_helpers.h  $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-dlg.obj: mcast-receiver-dlg.c pcc.h dsoundplay.h mcast_setup.h mcast-settings.h mcast-receiver-state-machine.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-dlg.obj: mcast-sender-dlg.c sender-settings-dlg.h mcast-sender-state-machine.h pcc.h mcast-sender-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-settings.obj: mcast-settings.c mcast-settings.h pcc.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-settings-dlg.obj: mcast-settings-dlg.c mcast-settings-dlg.h pcc.h resource.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\sender-settings-dlg.obj: sender-settings-dlg.c pcc.h sender-settings-dlg.h mcast-sender-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-settings.obj: mcast-sender-settings.c pcc.h mcast-sender-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\receiver-settings.obj: receiver-settings.c pcc.h receiver-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\receiver-settings-dlg.obj: receiver-settings-dlg.c pcc.h receiver-settings.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\wave_utils.obj: wave_utils.c pcc.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\dsoundplay.obj: dsoundplay.cpp input-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcpp.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcpp.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\message-loop.obj: message-loop.c message-loop.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\fifo-circular-buffer.obj: fifo-circular-buffer.c fifo-circular-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\input-buffer.obj: input-buffer.c input-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-input-buffer.obj: ut-input-buffer.c input-buffer.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\ut-fifo-circular-buffer.obj: ut-fifo-circular-buffer.c $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

# Tests
$(OUTDIR)\ut-fifo-circular-buffer.exe: $(OUTDIR_OBJ)\fifo-circular-buffer.obj $(OUTDIR_OBJ)\ut-fifo-circular-buffer.obj $(OUTDIR_OBJ)\timeofday.obj
	@ECHO $@
	$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$@.map /PDB:$(OUTDIR_OBJ)\ut-fifo-circular-buffer.pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib
	
$(OUTDIR)\ut-input-buffer.exe: $(OUTDIR_OBJ)\input-buffer.obj $(OUTDIR_OBJ)\ut-input-buffer.obj 
	@ECHO $@
	$(link) $(ldebug) /nologo /SUBSYSTEM:console /LIBPATH:$(DXLIB) /MAP:$@.map /PDB:$(OUTDIR_OBJ)\ut-fifo-circular-buffer.pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\receiver.exe: $(OUTDIR_OBJ)\mcastui.res $(OUTDIR_OBJ)\mcast-receiver-dlg.obj $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\input-buffer.obj $(OUTDIR_OBJ)\fifo-circular-buffer.obj $(OUTDIR_OBJ)\dsoundplay.obj $(OUTDIR_OBJ)\mcast_setup.obj $(OUTDIR_OBJ)\winsock_adapter.obj $(OUTDIR_OBJ)\wave_utils.obj $(OUTDIR_OBJ)\mcast_utils.obj $(OUTDIR_OBJ)\timeofday.obj $(OUTDIR_OBJ)\resolve.obj $(OUTDIR_OBJ)\message-loop.obj  $(OUTDIR_OBJ)\mcast-receiver-state-machine.obj $(OUTDIR_OBJ)\receiver-settings.obj $(OUTDIR_OBJ)\receiver-settings-dlg.obj $(OUTDIR_OBJ)\mcast-settings.obj $(OUTDIR_OBJ)\mcast-settings-dlg.obj
	@ECHO $@
	$(link) $(ldebug) $(guiflags) /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\sender.exe: $(OUTDIR_OBJ)\mcastui.res $(OUTDIR_OBJ)\timeofday.obj $(OUTDIR_OBJ)\mcast_utils.obj $(OUTDIR_OBJ)\resolve.obj $(OUTDIR_OBJ)\mcast_setup.obj $(OUTDIR_OBJ)\mcast-sender-dlg.obj $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\message-loop.obj $(OUTDIR_OBJ)\wave_utils.obj $(OUTDIR_OBJ)\mcast-sender-state-machine.obj $(OUTDIR_OBJ)\sender-settings-dlg.obj $(OUTDIR_OBJ)\mcast-sender-settings.obj $(OUTDIR_OBJ)\mcast-settings.obj  $(OUTDIR_OBJ)\mcast-settings-dlg.obj
	@ECHO $@
	$(link) $(ldebug) $(guiflags) /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$(OUTDIR)\$(@B).map -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
	$(CLEANUP)
