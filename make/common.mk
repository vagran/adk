# /ADK/make/common.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

################################################################################

ifeq ($(ADK_APP_TYPE),unit_test)
    # Unit test

    ADK_PLATFORM ?= native
    
    ADK_PLATFORM_MAKEFILE = unit_test.mk
    ADK_APP_NAME = $(ADK_TEST_NAME)
    ADK_BUILD_TYPE = debug
    
    # Compilation tools.
    CC = $(NAT_CC)
    LD = $(NAT_LD)
    NM = $(NAT_NM)
    OBJCOPY = $(NAT_OBJCOPY)
    CPPFILT = $(NAT_CPPFILT)
    
else ifeq ($(ADK_APP_TYPE),doc)
    # Documentation
    ADK_PLATFORM_MAKEFILE = doc.mk
else ifeq ($(ADK_PLATFORM),avr)
    ADK_PLATFORM_MAKEFILE = avr.mk
else
    ADK_PLATFORM_MAKEFILE = desktop.mk
endif

# Verify target if application name is specified.
ifdef ADK_APP_NAME

    ifeq ($(ADK_BUILD_TYPE),release)
    
    else ifeq ($(ADK_BUILD_TYPE),debug)
        DEFS += DEBUG
    else
        $(error Build type not supported: $(ADK_BUILD_TYPE))
    endif
    
    ifeq ($(ADK_PLATFORM),native)
        ADK_PLATFORM = $(ADK_NAT_PLATFORM)
    endif
    
    ifeq ($(ADK_PLATFORM),avr)
        ADK_PLATFORM_ID = $(ADK_PLATFORM_ID_AVR)
        DEFS += ADK_PLATFORM_AVR
        # Compilation tools.
        CC = $(AVR_CC)
        LD = $(AVR_LD)
        OBJCOPY = $(AVR_OBJCOPY)
        OBJDUMP = $(AVR_OBJDUMP)
        SIZE = $(AVR_SIZE)
    else ifeq ($(ADK_PLATFORM),linux32)
        ADK_PLATFORM_ID = $(ADK_PLATFORM_ID_LINUX32)
        DEFS += ADK_PLATFORM_LINUX32
        # Compilation tools.
        CC = $(LINUX32_CC)
        LD = $(LINUX32_LD)
        NM = $(LINUX32_NM)
        OBJCOPY = $(LINUX32_OBJCOPY)
        CPPFILT = $(LINUX32_CPPFILT)
        SIZE = $(LINUX32_SIZE)
        # Format of object files
        OBJ_FORMAT = elf32-i386
        # Binary architecture of object files
        OBJ_ARCH = i386
        LIB_DIRS += /lib /lib/tls/i686/cmov
        ADK_USE_GUI ?= yes
    else ifeq ($(ADK_PLATFORM),linux64)
        ADK_PLATFORM_ID = $(ADK_PLATFORM_ID_LINUX64)
        DEFS += ADK_PLATFORM_LINUX64
        # Compilation tools.
        CC = $(LINUX64_CC)
        LD = $(LINUX64_LD)
        NM = $(LINUX64_NM)
        OBJCOPY = $(LINUX64_OBJCOPY)
        CPPFILT = $(LINUX64_CPPFILT)
        SIZE = $(LINUX64_SIZE)
        # Format of object files
        OBJ_FORMAT = elf64-x86-64
        # Binary architecture of object files
        OBJ_ARCH = i386
        LIB_DIRS += /lib /usr/lib64
        ADK_USE_GUI ?= yes
    else ifeq ($(ADK_PLATFORM),win32)
        ADK_PLATFORM_ID = $(ADK_PLATFORM_ID_WIN32)
        DEFS += ADK_PLATFORM_WIN32
    else ifeq ($(ADK_PLATFORM),win64)
        ADK_PLATFORM_ID = $(ADK_PLATFORM_ID_WIN64)
        DEFS += ADK_PLATFORM_WIN64
    else
        $(error Platform not supported: $(ADK_PLATFORM))
    endif
    
    # Application type is verified in specific platform makefile.

endif

# Use secondary expansion feature
.SECONDEXPANSION:

# Ensure default goal is the first declared one.
$(DEF_GOAL):

################################################################################
# Handle subdirectories
ifdef SUBDIRS

SUBDIRS_TARGET = $(foreach item,$(SUBDIRS),$(item).dir)
.PHONY: $(SUBDIRS_TARGET)
$(SUBDIRS_TARGET):
	@$(MAKE) -C $(patsubst %.dir,%,$@) $(if $(MAKECMDGOALS), $(MAKECMDGOALS), $(DEF_GOAL))

$(MAKECMDGOALS): $(SUBDIRS_TARGET)

endif

################################################################################
# Utilities

# A literal space.
space :=
space +=
# Join elements in list specified in argument 2 using separator from argument 1.
JOIN = $(subst $(space),$1,$(strip $2))

# Produce arbitrary binary file object file creating command. Parameters:
# $1 - path to source binary file
# $2 - path to target object file
define EMBED_BINARY
if [ -z $$(dir $1) ]; then \
	$(OBJCOPY) -I binary -O $(OBJ_FORMAT) -B $(OBJ_ARCH) $1 $2; \
else \
	(cd $(dir $1) && $(OBJCOPY) -I binary -O $(OBJ_FORMAT) -B $(OBJ_ARCH) $(notdir $1) $2); \
fi;
endef

################################################################################
# Output directories

ifdef ADK_APP_NAME

# All files produced by build are placed there.
COMPILE_DIR = $(CURDIR)/build
# Directory for current target.
ADK_OBJ_DIR = $(COMPILE_DIR)/$(ADK_PLATFORM)-$(ADK_BUILD_TYPE)

$(ADK_OBJ_DIR):
	[ -d $@ ] || $(MKPATH) $@

.PHONY: adk_clean_obj_dir adk_clean_build_dir adk_remove_build_dir

# Use this variable as dependecy from build directory. It will expand to empty
# string if the directory already exists. This prevents from continuous rebuilds
# because the directory timestamp changes after content is modificated.
ADK_BUILD_DIR = $(filter-out $(wildcard $(ADK_OBJ_DIR)), $(ADK_OBJ_DIR))

adk_clean_obj_dir:
	$(RMPATH) $(ADK_OBJ_DIR)

adk_clean_build_dir: adk_clean_obj_dir
	$(if $(wildcard $(COMPILE_DIR)/*), , $(RMPATH) $(COMPILE_DIR))
	
adk_remove_build_dir:
	$(RMPATH) $(COMPILE_DIR)

clean: adk_clean_build_dir

clean_all: adk_remove_build_dir

# ADK_APP_NAME
endif

################################################################################
# Common compilation parameters

ifdef ADK_APP_NAME

# Precompiled headers
PCHS += $(ADK_ROOT)/include/adk.h
ADK_PCHS = $(foreach hdr, $(PCHS), $(ADK_OBJ_DIR)/$(notdir $(hdr)).gch)
ADK_DEPS += $(ADK_PCHS:.gch=.d)

# Default recipe should fail
ifndef ADK_GCH_RECIPE
define ADK_GCH_RECIPE
$(error PCH recipe is not defined)
endef
# ADK_GCH_RECIPE
endif

# PCH rule definition.
# $1 - PCH
# $2 - source header path
define PCH_RULE
$(1): $(2) $(ADK_BUILD_DIR)
	$$(ADK_GCH_RECIPE)
endef
$(foreach hdr,$(PCHS),$(eval $(call PCH_RULE,$(ADK_OBJ_DIR)/$(notdir $(hdr)).gch,$(hdr))))

INCLUDE_DIRS += $(ADK_ROOT)/include
# ADK_OBJ_DIR should be the first in include paths list in order to give 
# priority to precompiled headers.
IFLAGS += $(foreach dir, $(ADK_OBJ_DIR) $(INCLUDE_DIRS), -I$(dir))

DEFS += ADK_APP_NAME=$(ADK_APP_NAME) ADK_BUILD_TYPE=$(ADK_BUILD_TYPE) \
	ADK_PLATFORM=$(ADK_PLATFORM) ADK_PLATFORM_ID=$(ADK_PLATFORM_ID)

COMMON_COMP_FLAGS += $(WARN_COMP_FLAGS) $(foreach def, $(DEFS), -D$(def)) $(IFLAGS)

COMMON_CPP_FLAGS += -std=$(CPP_STD)

COMMON_C_FLAGS += -std=$(C_STD)

LIB_FLAGS += $(foreach lib, $(LIBS), -l$(lib))

LIB_FLAGS += $(foreach file, $(LIB_DIRS), -L$(file))

# ADK_APP_NAME
endif

.PHONY: all clean clean_all test install

# Default definition for testing target
test:

# Default definition for installation target
install:

################################################################################
# Used features

ifeq ($(ADK_USE_GUI),yes)
DEFS += ADK_USE_GUI
DESKTOP_LIBS += $(DESKTOP_GUI_LIBS)
DESKTOP_LIBS_INC_DIRS += $(DESKTOP_GUI_LIBS_INC_DIRS)
endif 

ifeq ($(ADK_USE_PYTHON),yes)
DEFS += ADK_USE_PYTHON
DESKTOP_LIBS += $(PYTHON_LIB)
DESKTOP_LIBS_INC_DIRS += $(PYTHON_INC_DIR)
endif

################################################################################
# Common compilation rules

ifdef ADK_APP_NAME

# Substitute multiple patterns by one
PAT_SUBST = $(foreach pat, $(1), $(patsubst $(pat), $(2), $(filter $(pat), $(3))))

# Scan directories for files using specified patterns
SCAN_PATH = $(foreach pat, $(2), $(wildcard $(1)/$(pat)))

# Get object files based on source files in specified directory
GET_OBJ = $(call PAT_SUBST, %.c %.cpp %.S, %.o, $(notdir $(call SCAN_PATH, $(1), *.c *.cpp *.S)))

# Add source files from current directory
SRC_DIRS += .

# Scan all specified additional directories
OBJS += $(foreach src_dir, $(SRC_DIRS), $(call GET_OBJ, $(src_dir)))

# Add all specified additional source files
SRC_DIRS += $(dir $(SRCS))
OBJS += $(call PAT_SUBST, %.c %.cpp %.S, %.o, $(notdir $(SRCS)))

ADK_OBJS = $(foreach obj, $(sort $(OBJS)), $(ADK_OBJ_DIR)/$(obj))
ADK_SRC_DIRS = $(sort $(SRC_DIRS))
# Additional automatic dependencies for object files
ADK_DEPS += $(ADK_OBJS:.o=.d)

VPATH += $(ADK_SRC_DIRS)

# include dependencies if exist
-include $(ADK_DEPS)

# ADK_APP_NAME
endif

################################################################################
# Raw resources embedded into the executable binary

# Create object files from resource files
RES_OBJ_FILES = $(foreach file, $(sort $(RES_FILES)), $(ADK_OBJ_DIR)/$(notdir $(file).res.o))
RES_HDR_FILES = $(foreach file, $(sort $(RES_FILES)), $(ADK_OBJ_DIR)/$(notdir $(file).res.h))
ADK_OBJS += $(RES_OBJ_FILES)

# Header file with definition of resources location in data section
$(ADK_OBJ_DIR)/%.res.h: % $(ADK_BUILD_DIR)
	echo "/* This file is generated automatically from \"$<\" file. */" > $@
	echo "extern \"C\" const char _binary_$(notdir $(subst .,_,$<))_start;" >> $@
	echo "extern \"C\" const char _binary_$(notdir $(subst .,_,$<))_end;" >> $@
	echo "ADK_DECL_RESOURCE($(notdir $(subst .,_,$<)), \"$(notdir $<)\", \
&_binary_$(notdir $(subst .,_,$<))_start, \
&_binary_$(notdir $(subst .,_,$<))_end)" >> $@

# Object file from resource file
$(ADK_OBJ_DIR)/%.res.o: % $(ADK_BUILD_DIR)
	$(call EMBED_BINARY, $<, $@)

RES_AUTO_HDR = $(ADK_OBJ_DIR)/auto_adk_res.h

$(ADK_OBJS): $(RES_AUTO_HDR)

# Main ADK include file should depend on resources automatic header in order to
# make pre-compiled header.
$(ADK_ROOT)/include/adk.h: $(RES_AUTO_HDR)

# Header file which includes all automatic resource header files
$(RES_AUTO_HDR): $$(RES_HDR_FILES) $(ADK_BUILD_DIR)
	echo "/* This file is generated automatically. */" > $@
	$(foreach file, $(RES_HDR_FILES), echo "#include <$(notdir $(file))>" >> $@;)

################################################################################
# Include platform specific makefile

ifdef ADK_PLATFORM_MAKEFILE
    include $(ADK_ROOT)/make/$(ADK_PLATFORM_MAKEFILE)
endif

$(ADK_OBJS): $$(ADK_PCHS) $(ADK_BUILD_DIR)
