#----- Include the PSDK's WIN32.MAK to pick up defines------------------------------------
!include "obj.mk"
APP_NAME=mcastui
#----- OUTDIR_OBJ is defined in WIN32.MAK This is the name of the destination directory-------
        
all: $(OUTDIR)\$(APP_NAME).exe $(OUTDIR)\ut-fifo-circular-buffer.exe $(OUTDIR)\ut-input-buffer.exe $(OUTDIR)\sender.exe $(OUTDIR)\receiver.exe
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

$(OUTDIR_OBJ)\mcastui.res: mcastui.rc resource.h mcastui.h $(OUTDIR_OBJ)
	$(rc) $(rcflags) $(rcvars) /fo $(OUTDIR_OBJ)\mcastui.res %s

$(OUTDIR_OBJ)\mcast_utils.obj: mcast_utils.c mcast_utils.h resolve.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\resolve.obj: resolve.c resolve.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\timeofday.obj: timeofday.c timeofday.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\winsock_adapter.obj: winsock_adapter.c timeofday.h winsock_adapter.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\var-database.obj: var-database.c var-database.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-state-machine.obj: mcast-sender-state-machine.c mcast-sender-state-machine.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-state-machine.obj: mcast-receiver-state-machine.c mcast-receiver-state-machine.h $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\conn_table.obj: conn_table.c conn_table.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags)  /WX /W3 /c /nologo /W3 /WX /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\audio-buffer.obj: audio-buffer.c audio-buffer.h pcc.h $(OUTDIR_OBJ) $(OUTDIR_PCC)\pcc.pch
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\conn_data.obj: conn_data.c conn_data.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /c /nologo /W3 /WX /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\tei.obj: tei.c tei.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /c /nologo /W3 /WX /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast_setup.obj: mcast_setup.c winsock_adapter.h mcast_setup.h conn_data.h mcast_utils.h winsock_adapter.h resolve.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\debug_helpers.obj: debug_helpers.c pcc.h debug_helpers.h  $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcastui.obj: mcastui.c pcc.h mcastui.h dsoundplay.h conn_data.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-receiver-dlg.obj: mcast-receiver-dlg.c pcc.h dsoundplay.h conn_data.h $(OUTDIR_OBJ)
    $(cc) $(cdebug) $(cvars) $(cflags) /W3 /WX /Yupcc.h /Fp$(OUTDIR_PCC)\pcc.pch /Fo"$(OUTDIR_OBJ)\\" /Fd"$(OUTDIR_OBJ)\\" %s

$(OUTDIR_OBJ)\mcast-sender-dlg.obj: mcast-sender-dlg.c pcc.h dsoundplay.h conn_data.h $(OUTDIR_OBJ)
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

# Build rule for EXE
$(OUTDIR)\$(APP_NAME).exe: $(OBJECTS)
	@ECHO $@
	$(link) $(ldebug) $(guiflags) /LIBPATH:$(DXLIB) /MAP:$@.map /PDB:$(OUTDIR_OBJ)\$(APP_NAME).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\receiver.exe: $(OUTDIR_OBJ)\mcastui.res $(OUTDIR_OBJ)\mcast-receiver-dlg.obj $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\input-buffer.obj $(OUTDIR_OBJ)\fifo-circular-buffer.obj $(OUTDIR_OBJ)\dsoundplay.obj $(OUTDIR_OBJ)\mcast_setup.obj $(OUTDIR_OBJ)\winsock_adapter.obj $(OUTDIR_OBJ)\wave_utils.obj $(OUTDIR_OBJ)\conn_data.obj $(OUTDIR_OBJ)\mcast_utils.obj $(OUTDIR_OBJ)\conn_table.obj $(OUTDIR_OBJ)\tei.obj $(OUTDIR_OBJ)\timeofday.obj $(OUTDIR_OBJ)\resolve.obj $(OUTDIR_OBJ)\var-database.obj $(OUTDIR_OBJ)\message-loop.obj  $(OUTDIR_OBJ)\mcast-receiver-state-machine.obj   
	@ECHO $@
	$(link) $(ldebug) $(guiflags) /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$@.map /PDB:$(OUTDIR_OBJ)\$(APP_NAME).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

$(OUTDIR)\sender.exe: $(OUTDIR_OBJ)\mcastui.res $(OUTDIR_OBJ)\tei.obj $(OUTDIR_OBJ)\conn_table.obj $(OUTDIR_OBJ)\conn_data.obj $(OUTDIR_OBJ)\timeofday.obj $(OUTDIR_OBJ)\mcast_utils.obj $(OUTDIR_OBJ)\resolve.obj $(OUTDIR_OBJ)\mcast_setup.obj $(OUTDIR_OBJ)\mcast-sender-dlg.obj $(OUTDIR_OBJ)\debug_helpers.obj $(OUTDIR_OBJ)\message-loop.obj $(OUTDIR_OBJ)\wave_utils.obj $(OUTDIR_OBJ)\mcast-sender-state-machine.obj
	@ECHO $@
	$(link) $(ldebug) $(guiflags) /MACHINE:X86 /LIBPATH:$(DXLIB) /MAP:$@.map /PDB:$(OUTDIR_OBJ)\$(APP_NAME).pdb -out:$@ $** $(guilibs) ComCtl32.lib dsound.lib winmm.lib dxguid.lib ole32.lib

#--------------------- Clean Rule --------------------------------------------------------
# Rules for cleaning out those old files
clean:
	$(CLEANUP)
