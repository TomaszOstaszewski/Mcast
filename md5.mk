# This is MD5 definitions part.

# MD5 checksums are generated and checked for every dependency on the target
# dependency list. Specify NOMD5=1 at make invocation line to disable it.
# This can speed up one-time build, but there's no guarantee that incremental
# build will be reliable.

# Where to store md5 sums.
MD5DIR := $(BUILD_ROOT)md5/

# Function for requesting dependency on md5 checksum in addition to standard
# file dependency.
define to-md5
$(1) $(if $(findstring $(NOMD5),1),,$(addprefix $(MD5DIR),$(addsuffix .md5,$(patsubst ../%,%,$(1)))))
endef

# This is a variant of rule above. It makes target dependent on already
# generated md5 checksums only. This is used when generating dependency files.
define to-existing-md5
$(1) $(if $(findstring $(NOMD5),1),,$(realpath $(addprefix $(MD5DIR),$(addsuffix .md5,$(patsubst ../%,%,$(1))))))
endef

# Calculation of md5 checksum.
# Based on http://www.cmcrossroads.com/ask-mr-make/6795-rebuilding-when-a-files-checksum-changes
define calculate-md5
$(if $(filter-out $(shell cat $@ 2>/dev/null),$(shell md5sum $<)),md5sum $< > $@)
endef

.PRECIOUS: $(MD5DIR)%.md5

$(MD5DIR)%.md5: % FORCE | $$(@D)/.
	$(NOECHO)$(calculate-md5)

$(MD5DIR)%.md5: ../% FORCE | $$(@D)/.
	$(NOECHO)$(calculate-md5)

