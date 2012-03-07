# /ADK/make/common.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

# Verify target if application name is specified.
ifdef ADK_APP_NAME

    ifdef SUBDIRS
        $(error 'SUBDIRS' cannot be specified in application directory)
    endif

    ifeq ($(ADK_BUILD_TYPE),release)
    
    else ifeq ($(ADK_BUILD_TYPE),debug)
    
    else
        $(error Target not supported: $(ADK_BUILD_TYPE))
    endif
    
    ifeq ($(ADK_PLATFORM),avr)
    
    else ifeq ($(ADK_PLATFORM),debug)
    
    else ifeq ($(ADK_PLATFORM),linux32)
    
    else ifeq ($(ADK_PLATFORM),linux64)
    
    else ifeq ($(ADK_PLATFORM),win32)
    
    else ifeq ($(ADK_PLATFORM),win64)
    
    else
        $(error Platform not supported: $(ADK_PLATFORM))
    endif
    
    # Application type is verified in specific platform makefile.

endif

# All files produced by build are placed there.
COMPILE_DIR = $(CURDIR)/build
# Directory for current target.
OBJ_DIR = $(COMPILE_DIR)/$(ADK_PLATFORM)-$(ADK_BUILD_TYPE)

# Handle subdirectories
ifdef SUBDIRS

SUBDIRS_TARGET = $(foreach item,$(SUBDIRS),$(item).dir)
.PHONY: $(SUBDIRS_TARGET)
$(SUBDIRS_TARGET):
	@$(MAKE) -C $(patsubst %.dir,%,$@) $(MAKECMDGOALS)

$(MAKECMDGOALS): $(SUBDIRS_TARGET)

endif

# Common phony targets
.PHONY: all clean

# XXX clean target definition
