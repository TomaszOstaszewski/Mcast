include md5.mk

EXTRA_DEP=$$(@D)/$$*.d $$(@D)/$$*.o $$(@D)/$$*.S $$(@D)/$$*.def dirs-$(1) doxy-$(1) bundle log
# This variable says whether or not
# paths for the standard library files shall be
# present in dependencies.
STDLIB_PATH_IN_DEP:=0

# if DEP is set to value 0
# then no dependencies will be generated.
ifeq ($(DEP),0)
INCLUDE_PATH=$$($(1)_INCLUDES)
DEPGEN_FLAGS   =
endif

TRANSFORM_STRING	='$$(@D)/$$*$$(suffix $$<) $(EXTRA_DEP)'

# if DEP is set to value 1
# then dependencies will be generated as components needed
# to perform actual compilation.
ifeq ($(DEP),1)
INCLUDE_PATH=$$(addprefix -I,$$(sort $$(^D))) 
DEPGEN_FLAGS	 =
# For regular dependency generation, the flag has to be -MM
# This is because the include dirs for object file generation
# are taken via '$$(^D)' rule. Hence, with -M, the include
# dirs string would be cluttered with standard library
# directories. Placing explicit include directories causes
# a compilation to fail in this case.
endif

# if DEP is set to value 2
# then dependencies will be generated as a byproduct
# of regular compilation.
ifeq ($(DEP),2)
# If STDLIB_PATH_IN_DEP is set, then pass the -MD flag.
# This causes also the standard library files to be placed into
# dependencies. Otherwise, just pass -MMD flag, which omits the 
# standard library files from dependencies.
ifeq ($(STDLIB_PATH_IN_DEP),0)
DEPGEN_FLAG=-MMD
else
DEPGEN_FLAG=-MD
endif
INCLUDE_PATH=$$($(1)_INCLUDES)
DEPGEN_FLAGS   = $(DEPGEN_FLAG) -MP -MF $$(@D)/$$*.d -MT $(TRANSFORM_STRING)
endif

define SETUP_VARS
$(1)_OBJ_DIR:=$(BUILD_ROOT)$(1)/
$(1)_SRC:=$$(addprefix $$($(1)_DIR),$$($(1)_FILES))

$(1)_OBJ+=$$(addprefix $$($(1)_OBJ_DIR),$$(patsubst %.c,%.o,   $$(filter %.c,   $$($(1)_FILES))))
$(1)_OBJ+=$$(addprefix $$($(1)_OBJ_DIR),$$(patsubst %.cc,%.o,  $$(filter %.cc,  $$($(1)_FILES))))
$(1)_OBJ+=$$(addprefix $$($(1)_OBJ_DIR),$$(patsubst %.cpp,%.o, $$(filter %.cpp, $$($(1)_FILES))))

$(1)_DEP:=$$(patsubst %.o,%.d,$$($(1)_OBJ))
$(1)_GCOV+=$$(patsubst %.o,%.gcda,$$($(1)_OBJ))
$(1)_GCOV+=$$(patsubst %.o,%.gcno,$$($(1)_OBJ))

ALL_SRC+=$$($(1)_SRC)
$$($(1)_TARGET)_OBJ+=$$($(1)_OBJ)
ifneq ($$($(1)_TARGET),)
ALL_BINARIES+=$(addprefix $(BUILD_ROOT),$$($(1)_TARGET))
endif
ALL_GCOV+=$$($(1)_GCOV)
ALL_OBJECTS+=$$($(1)_OBJ)
ALL_DEPENDS+=$$($(1)_DEP)
ALL_INCLUDES+=$$(sort $$($(1)_INCLUDES))

.PHONY : dirs-$(1)
dirs-$(1):
	@echo $$(sort $$(^D))

$$(BUILD_ROOT)$$($(1)_TARGET) : $$($(1)_OBJ) $$($(1)_LIB) | $$$$(@D)/. $$($(1)_ORDER_ONLY_DEP)
	@$(ECHO_LD) $$@
	$(NOECHO)$(CXX) -o $$@ $$^ $$($(1)_LDFLAGS) $$($(1)_LIB) $$(CFLAGS)

endef

define MAKE_DEPEND
$$($(1)_OBJ_DIR)%.d: $$$$(call to-existing-md5,$$($(1)_DIR)%.c $$$$^) | $$$$(@D)/.
	@$(ECHO_DEP) $$@
	$(NOECHO)$(CC)  $(CPPFLAGS) $$($(1)_CPPFLAGS) $$($(1)_INCLUDES) -MM -MF $$@ -MT $(TRANSFORM_STRING) $$<
$$($(1)_OBJ_DIR)%.d: $$$$(call to-existing-md5,$$($(1)_DIR)%.cpp $$$$^) | $$$$(@D)/.
	@$(ECHO_DEP) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $$($(1)_CPPFLAGS) $$($(1)_INCLUDES) -MM -MF $$@ -MT $(TRANSFORM_STRING) $$<
$$($(1)_OBJ_DIR)%.d: $$$$(call to-existing-md5,$$($(1)_DIR)%.cc $$$$^) | $$$$(@D)/.
	@$(ECHO_DEP) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $$($(1)_CPPFLAGS) $$($(1)_INCLUDES) -MM -MF $$@ -MT $(TRANSFORM_STRING) $$<
endef

define 	MAKE_OBJECT
# Rules for creating object files
$$($(1)_OBJ_DIR)%.o: $$$$(call to-md5,$$($(1)_DIR)%.c $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CC)  $(CPPFLAGS) $(INCLUDE_PATH) $(DEPGEN_FLAGS) $$($(1)_CPPFLAGS) $(CFLAGS) $$($(1)_CFLAGS) -c -o $$@ $$<
$$($(1)_OBJ_DIR)%.o: $$$$(call to-md5,$$($(1)_DIR)%.cpp $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $(INCLUDE_PATH) $(DEPGEN_FLAGS) $$($(1)_CPPFLAGS) $(CFLAGS) $$($(1)_CFLAGS) -c -o $$@ $$<
$$($(1)_OBJ_DIR)%.o: $$$$(call to-md5,$$($(1)_DIR)%.cc $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $(INCLUDE_PATH) $(DEPGEN_FLAGS) $$($(1)_CPPFLAGS) $(CFLAGS) $$($(1)_CFLAGS) -c -o $$@ $$<

# Rules for creating preprocessor source files
$$($(1)_OBJ_DIR)%.c: $$$$(call to-md5,$$($(1)_DIR)%.c $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CC)   $(CPPFLAGS) $(INCLUDE_PATH) -E $$($(1)_CPPFLAGS) -o $$@ $$<
$$($(1)_OBJ_DIR)%.cpp: $$$$(call to-md5,$$($(1)_DIR)%.cpp $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX)  $(CPPFLAGS) $(INCLUDE_PATH) -E $$($(1)_CPPFLAGS) -o $$@ $$<
$$($(1)_OBJ_DIR)%.cc: $$$$(call to-md5,$$($(1)_DIR)%.cc $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX)  $(CPPFLAGS) $(INCLUDE_PATH) -E $$($(1)_CPPFLAGS) -o $$@ $$<

# Rules for creating preprocessor definition files
$$($(1)_OBJ_DIR)%.def: $$$$(call to-md5,$$($(1)_DIR)%.c $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CC)  $(CPPFLAGS) $(INCLUDE_PATH) -E -dM $$($(1)_CPPFLAGS) -o $$@ $$<
$$($(1)_OBJ_DIR)%.def: $$$$(call to-md5,$$($(1)_DIR)%.cpp $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $(INCLUDE_PATH) -E -dM $$($(1)_CPPFLAGS) -o $$@ $$<
$$($(1)_OBJ_DIR)%.def: $$$$(call to-md5,$$($(1)_DIR)%.cc $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $(INCLUDE_PATH) -E -dM $$($(1)_CPPFLAGS) -o $$@ $$<

# Rules for creating assembly files
$$($(1)_OBJ_DIR)%.S: $$$$(call to-md5,$$($(1)_DIR)%.c $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CC)  $(CPPFLAGS) $(INCLUDE_PATH) $(DEPGEN_FLAGS) $$($(1)_CPPFLAGS) $(CFLAGS) $$($(1)_CFLAGS) -S -o $$@ $$<
$$($(1)_OBJ_DIR)%.S: $$$$(call to-md5,$$($(1)_DIR)%.cpp $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $(INCLUDE_PATH) $(DEPGEN_FLAGS) $$($(1)_CPPFLAGS) $(CFLAGS) $$($(1)_CFLAGS) -S -o $$@ $$<
$$($(1)_OBJ_DIR)%.S: $$$$(call to-md5,$$($(1)_DIR)%.cc $$$$^) | $$$$(@D)/.
	@$(ECHO_CC) $$@
	$(NOECHO)$(CXX) $(CPPFLAGS) $(INCLUDE_PATH) $(DEPGEN_FLAGS) $$($(1)_CPPFLAGS) $(CFLAGS) $$($(1)_CFLAGS) -S -o $$@ $$<
endef 

define MAKE_ALL_DEPEND_ON_COMPONENT
all : $$(addprefix $$(BUILD_ROOT),$$($(1)_TARGET)) 
endef

