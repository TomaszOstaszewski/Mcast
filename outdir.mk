!include <win32.mak>
!include "obj.mk"
#----- OUTDIR is defined in WIN32.MAK This is the name of the destination directory-------

$(OUTDIR):
    @if not exist "$(OUTDIR_OBJ)/$(NULL)" mkdir $(OUTDIR_OBJ)

$(OUTDIR_PCC) : $(OUTDIR)
    @if not exist "$(OUTDIR_PCC)/$(NULL)" mkdir $(OUTDIR_PCC)

$(OUTDIR_OBJ) : $(OUTDIR)
    @if not exist "$(OUTDIR_OBJ)/$(NULL)" mkdir $(OUTDIR_OBJ)
