# An ultimate makefile... 
#
#

.SUFFIXES:
.DEFAULT_GOAL:=all
.PHONY : clean depclean realclean cscope ctags log tests run-tests

include eyecandy.mk

.SECONDEXPANSION:

BUILD_ROOT:=$(firstword $(subst -, ,$(shell $(CC) -dumpmachine)))/
APP_NAME:=app
DEP		?=2

CPPFLAGS?=-D_GNU_SOURCE 
CFLAGS	?=-fprofile-arcs -ftest-coverage -ffunction-sections -fdata-sections -ggdb -O0 -Wall
LDFLAGS	:=-fprofile-arcs -ftest-coverage -Wl,--Map=$(BUILD_ROOT)$(APP_NAME).map
NOMD5:=1

TOOLS_DIR:=$(BUILD_ROOT)tools/
CHECK_SRC:=check-0.9.8.tgz
CHECK_VER:=$(basename $(CHECK_SRC))
CHECK_DIR:=$(BUILD_ROOT)tools/
CHECK_BUILD_DIR:=$(addprefix /tmp/,$(USER))/
CHECK_HEADERS:=$(TOOLS_DIR)include/
LIBUT_SUFFIX:=ut
LIBUT_DIR:=$(TOOLS_DIR)lib/
LIBUT_NAME:=$(addprefix lib,$(addsuffix .a,$(LIBUT_SUFFIX)))
LIBUT:=$(LIBUT_DIR)$(LIBUT_NAME)

.PRECIOUS: brick_mcast_sm.c brick_mcast_sm.o

main_DIR:=./
main_FILES:=\
 brick-main.c\
 brick_mcast_sm.c
main_INCLUDES:=-I./smc_6_0_1/lib/C/
main_TARGET:=$(APP_NAME)

COMPONENTS:= \
 main\

UT_COMPONENTS:= \

.PHONY : ut

include autodir.mk
include guts.mk
include sm.mk

$(foreach comp,$(COMPONENTS) $(UT_COMPONENTS),$(eval $(call SETUP_VARS,$(comp))))

ifeq ($(DEP),0)
else
ifeq ($(DEP),1)
$(foreach comp,$(COMPONENTS) $(UT_COMPONENTS),$(eval $(call MAKE_DEPEND,$(comp))))
ifeq ($(filter %clean,$(MAKECMDGOALS)),)
include $(ALL_DEPENDS)
endif
else
ifeq ($(filter %clean,$(MAKECMDGOALS)),)
-include $(ALL_DEPENDS)
endif
endif
endif

$(foreach comp,$(COMPONENTS) $(UT_COMPONENTS),$(eval $(call MAKE_OBJECT,$(comp))))

$(CHECK_HEADERS)check.h : $(CHECK_SRC) | $$(@D)/.
	@$(ECHO_UNPACK) $<
	$(NOECHO)mkdir -p $(CHECK_BUILD_DIR); tar -C $(CHECK_BUILD_DIR) -xf $<	
	@$(ECHO_CONFIGURE) $(CHECK_VER)
	$(NOECHO)cd $(CHECK_BUILD_DIR)$(CHECK_VER); ./configure --prefix=$(abspath $(TOOLS_DIR)) --datarootdir=$(CHECK_BUILD_DIR) --quiet
	@$(ECHO_MAKE) $(CHECK_VER)
	$(NOECHO)cd $(CHECK_BUILD_DIR)$(CHECK_VER); $(MAKE) install --no-print-directory --quiet; $(RM) -r $(CHECK_BUILD_DIR)

# If dependencies are not calculated automatically, then
# add explicit dependencies on generated header files.
ifeq ($(DEP),2)
$(main_OBJ_DIR)brick-main.o\
$(main_OBJ_DIR)brick_mcast_sm.o\
 : brick_mcast_sm.h
$(ut_0_OBJ) \
$(ut_1_OBJ) \
$(ut_2_OBJ) \
$(ut_3_OBJ) \
$(ut_stubs_OBJ) \
 : $(CHECK_HEADERS)check.h
endif

# Calling this macro causes the phony target 'all' to depend on all the
# component files.
$(foreach comp,$(COMPONENTS) $(UT_COMPONENTS),$(eval $(call MAKE_ALL_DEPEND_ON_COMPONENT,$(comp))))

# Rule to create a libut.a helper library
# TODO: GNU make archive member rules can be used here
$(LIBUT): $(ut_stubs_OBJ)
	@$(ECHO_AR) $@
	$(NOECHO)ar rcs $@ $^

define MAKE_UT

.PHONY : run-$(1)
tests : $$(addprefix $$(BUILD_ROOT),$$($(1)_TARGET)) 

run-$(1): $$(addprefix $$(BUILD_ROOT),$$($(1)_TARGET)) 
	@$(ECHO_UT) $$<
	$(NOECHO)$$<

run-tests : run-$(1)

endef

# Finally, make UT execute always
all : run-tests

# Create rules for compiling and executing Unit test only.
$(foreach comp,$(UT_COMPONENTS),$(eval $(call MAKE_UT,$(comp))))

$(BUILD_ROOT)cscope.files: $(MAKEFILE_LIST) | $(BUILD_ROOT)/.
	@$(ECHO_GENERATE) $@
	$(NOECHO)echo $(ALL_SRC) > $@
	$(NOECHO)sed -i -e 's, ,\n,g' $@

cscope: $(BUILD_ROOT)cscope.files
	@$(ECHO_GENERATE) $@
	$(NOECHO)cscope -bq -i $<

ctags: $(BUILD_ROOT)cscope.files
	@$(ECHO_GENERATE) $@
	$(NOECHO)ctags -L $<

log: all 
	@echo $? > $(BUILD_ROOT)$@

clean: $(if $(findstring $(DEP),2),depclean,)
	@$(ECHO_RM) $(ALL_OBJECTS) $(ALL_BINARIES) $(ALL_GCOV) brick_mcast_sm.*
	-$(NOECHO)$(RM) $(ALL_OBJECTS) $(ALL_BINARIES) $(ALL_GCOV) brick_mcast_sm.*
depclean: 
	@$(ECHO_RM) $(ALL_DEPENDS)
	-$(NOECHO)$(RM) $(ALL_DEPENDS)
realclean: clean $(if $(findstring $(DEP),2),depclean,)
	@$(ECHO_RM) $(BUILD_ROOT)
	-$(NOECHO)$(RM) -r $(BUILD_ROOT)
