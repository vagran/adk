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
    
    else
        $(error Build type not supported: $(ADK_BUILD_TYPE))
    endif
    
    ifeq ($(ADK_PLATFORM),avr)
        ADK_PLATFORM_MAKEFILE = avr.mk
        # Compilation tools.
        CC = avr-gcc
        OBJCOPY = avr-objcopy
        OBJDUMP = avr-objdump
    else ifeq ($(ADK_PLATFORM),linux32)
        ADK_PLATFORM_MAKEFILE = desktop.mk
    else ifeq ($(ADK_PLATFORM),linux64)
        ADK_PLATFORM_MAKEFILE = desktop.mk
    else ifeq ($(ADK_PLATFORM),win32)
        ADK_PLATFORM_MAKEFILE = desktop.mk
    else ifeq ($(ADK_PLATFORM),win64)
        ADK_PLATFORM_MAKEFILE = desktop.mk
    else
        $(error Platform not supported: $(ADK_PLATFORM))
    endif
    
    # Application type is verified in specific platform makefile.

endif

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

INCLUDE_DIRS += $(ADK_ROOT)/include

IFLAGS = $(foreach dir, $(INCLUDE_DIRS), -I($dir))

.PHONY: all clean

################################################################################
# Include platform specific makefile

ifdef ADK_APP_NAME
    include $(ADK_ROOT)/make/$(ADK_PLATFORM_MAKEFILE)
endif
