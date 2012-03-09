# /ADK/make/avr.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

################################################################################
# Compilation parameters

ifndef ADK_MCU
    $(error Target MCU model is not defined)
endif

DEFS += ADK_MCU=$(ADK_MCU)

ifeq ($(ADK_BUILD_TYPE),release)
    ifndef RELEASE_OPT_FLAGS
        # Optimize for size by default
        COMMON_COMP_FLAGS += -Os
    else
        COMMON_COMP_FLAGS += $(RELEASE_OPT_FLAGS)
    endif
else ifeq ($(ADK_BUILD_TYPE),debug)
    ifndef DEBUG_OPT_FLAGS
        # Do not optimize by default
        COMMON_COMP_FLAGS += -O0
    else
        COMMON_COMP_FLAGS += $(DEBUG_OPT_FLAGS)
    endif
else
    $(error Build type not supported: $(ADK_BUILD_TYPE))
endif

COMMON_COMP_FLAGS += -mmcu=$(ADK_MCU)

################################################################################
# Executable binary

BINARY = $(ADK_APP_NAME).elf

all: $(BINARY)

$(BINARY): $(ADK_OBJS)
	@echo ADK_OBJS: $(ADK_OBJS)

$(ADK_OBJ_DIR)/%.o: %.c
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -o $@ $<

#XXX cpp S

################################################################################
# Binary converted to text formats understandable by most firmware uploaders.

.PHONY: text

# XXX
