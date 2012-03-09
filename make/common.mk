# /ADK/make/common.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

################################################################################
# Verify target if application name is specified.
ifdef ADK_APP_NAME

    ifdef SUBDIRS
        $(error 'SUBDIRS' cannot be specified in application directory)
    endif

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
        OBJCOPY = avr-objcopy
        OBJDUMP = avr-objdump
    else ifeq ($(ADK_PLATFORM),linux32)
        ADK_PLATFORM_MAKEFILE = desktop.mk
        DEFS += ADK_PLATFORM_LINUX32
    else ifeq ($(ADK_PLATFORM),linux64)
        ADK_PLATFORM_MAKEFILE = desktop.mk
        DEFS += ADK_PLATFORM_LINUX64
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
	if [ ! -d $@ ]; then $(MKPATH) $@; fi

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

INCLUDE_DIRS += $(ADK_ROOT)/include

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

# Make object file name from source file name
GET_OBJ = $(patsubst %.c, %.o, $(filter %.c, $(notdir $(1)))) \
	$(patsubst %.cpp, %.o, $(filter %.cpp, $(notdir $(1)))) \
	$(patsubst %.S, %.o, $(filter $.S, $(notdir $(1)))) \

# Scan directories for source files
SCAN_PATH = $(call GET_OBJ, $(wildcard $(1)/*.c)) \
	$(call GET_OBJ, $(wildcard $(1)/*.cpp)) \
	$(call GET_OBJ, $(wildcard $(1)/*.S)) \

# Firstly add source files from current directory
OBJS += $(call SCAN_PATH, .)

# Then scan all specified additional directories
OBJS += $(foreach src_dir, $(SRC_DIRS), $(call SCAN_PATH, $(src_dir)))

# Add all specified additional source files
SRC_DIRS += $(foreach src, $(SRCS), $(dir $(src)))
OBJS += $(foreach src, $(SRCS), $(call GET_OBJ $(src)))

ADK_OBJS = $(foreach obj, $(sort $(OBJS)), $(ADK_OBJ_DIR)/$(obj))
ADK_SRC_DIRS = $(sort $(SRC_DIRS))

VPATH += $(ADK_SRC_DIRS)

$(ADK_OBJS): $(ADK_OBJ_DIR)

# ADK_APP_NAME
endif

################################################################################
# Include platform specific makefile

ifdef ADK_APP_NAME
    include $(ADK_ROOT)/make/$(ADK_PLATFORM_MAKEFILE)
endif
