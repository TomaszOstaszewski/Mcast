!include <win32.mak>
#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------
OUTDIR_OBJ=$(OUTDIR)\obj
OUTDIR_PCC=$(OUTDIR)\pcc

OBJECTS=\
 $(OUTDIR_OBJ)\resolve.obj \
 $(OUTDIR_OBJ)\winsock_adapter.obj \
 $(OUTDIR_OBJ)\debug_helpers.obj \
 $(OUTDIR_OBJ)\mcastui.obj \
 $(OUTDIR_OBJ)\mcast_utils.obj \
 $(OUTDIR_OBJ)\timeofday.obj \
 $(OUTDIR_OBJ)\conn_data.obj \
 $(OUTDIR_OBJ)\conn_table.obj \
 $(OUTDIR_OBJ)\mcast_setup.obj \
 $(OUTDIR_OBJ)\tei.obj \
 $(OUTDIR_OBJ)\dsoundplay.obj \
 $(OUTDIR_OBJ)\wave_utils.obj \
 $(OUTDIR_OBJ)\fifo-circular-buffer.obj \
 $(OUTDIR_OBJ)\input-buffer.obj \
 $(OUTDIR_OBJ)\var-database.obj \
 $(OUTDIR_OBJ)\mcast-receiver-state-machine.obj \
 $(OUTDIR_OBJ)\mcastui.res

