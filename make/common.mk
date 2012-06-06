# /ADK/make/common.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

################################################################################
# Verify target if application name is specified.
ifdef ADK_APP_NAME

    ifeq ($(ADK_BUILD_TYPE),release)
    
    else ifeq ($(ADK_BUILD_TYPE),debug)
        DEFS += DEBUG
    else
        $(error Build type not supported: $(ADK_BUILD_TYPE))
    endif
    
    ifeq ($(ADK_PLATFORM),avr)
        ADK_PLATFORM_MAKEFILE = avr.mk
        DEFS += ADK_PLATFORM_AVR
        # Compilation tools.
        CC = avr-gcc
        LD = avr-ld
        OBJCOPY = avr-objcopy
        OBJDUMP = avr-objdump
    else ifeq ($(ADK_PLATFORM),linux32)
        ADK_PLATFORM_MAKEFILE = desktop.mk
        DEFS += ADK_PLATFORM_LINUX32
    else ifeq ($(ADK_PLATFORM),linux64)
        ADK_PLATFORM_MAKEFILE = desktop.mk
        DEFS += ADK_PLATFORM_LINUX64
        # Compilation tools.
        CC = $(NAT_CC)
		LD = $(NAT_LD)
		NM = $(NAT_NM)
		OBJCOPY = $(NAT_OBJCOPY)
		CPPFILT = $(NAT_CPPFILT)
		# Format of object files
		OBJ_FORMAT = elf64-x86-64
		# Binary architecture of object files
		OBJ_ARCH = i386
    else ifeq ($(ADK_PLATFORM),win32)
        ADK_PLATFORM_MAKEFILE = desktop.mk
        DEFS += ADK_PLATFORM_WIN32
    else ifeq ($(ADK_PLATFORM),win64)
        ADK_PLATFORM_MAKEFILE = desktop.mk
        DEFS += ADK_PLATFORM_WIN64
    else
        $(error Platform not supported: $(ADK_PLATFORM))
    endif
    
    # Application type is verified in specific platform makefile.

endif

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
# Output directories

ifdef ADK_APP_NAME

# All files produced by build are placed there.
COMPILE_DIR = $(CURDIR)/build
# Directory for current target.
ADK_OBJ_DIR = $(COMPILE_DIR)/$(ADK_PLATFORM)-$(ADK_BUILD_TYPE)

$(ADK_OBJ_DIR):
	[ -d $@ ] || $(MKPATH) $@

.PHONY: adk_build_dir adk_clean_obj_dir adk_clean_build_dir

adk_build_dir: $(ADK_OBJ_DIR)

all: adk_build_dir

adk_clean_obj_dir:
	$(RMPATH) $(ADK_OBJ_DIR)

adk_clean_build_dir: adk_clean_obj_dir
	$(if $(wildcard $(COMPILE_DIR)/*), , $(RMPATH) $(COMPILE_DIR))

clean: adk_clean_build_dir

# ADK_APP_NAME
endif

################################################################################
# Common compilation parameters

ifdef ADK_APP_NAME

INCLUDE_DIRS += $(ADK_ROOT)/include $(ADK_OBJ_DIR)

IFLAGS = $(foreach dir, $(INCLUDE_DIRS), -I$(dir))

DEFS += ADK_APP_NAME=$(ADK_APP_NAME) ADK_BUILD_TYPE=$(ADK_BUILD_TYPE) \
	ADK_PLATFORM=$(ADK_PLATFORM)

COMMON_COMP_FLAGS = $(WARN_COMP_FLAGS) $(foreach def, $(DEFS), -D$(def)) $(IFLAGS)

# ADK_APP_NAME
endif

.PHONY: all clean

################################################################################
# Common compilaion rules

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
ADK_DEPS = $(ADK_OBJS:.o=.d)

VPATH += $(ADK_SRC_DIRS)

# include dependencies if exist
-include $(ADK_DEPS)

# ADK_APP_NAME
endif

################################################################################
# Include platform specific makefile

ifdef ADK_APP_NAME
    include $(ADK_ROOT)/make/$(ADK_PLATFORM_MAKEFILE)
endif
