# /ADK/make/desktop.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

# Compiler optimization flags for debug build
ifndef DEBUG_OPT_FLAGS
DEBUG_OPT_FLAGS = -O0
endif

# Compiler optimization flags for release build
ifndef RELEASE_OPT_FLAGS
RELEASE_OPT_FLAGS = -O2
endif

ifeq ($(ADK_BUILD_TYPE),release)
	CFLAGS += $(RELEASE_OPT_FLAGS)
else ifeq ($(ADK_BUILD_TYPE),debug)
    CFLAGS += $(DEBUG_OPT_FLAGS) -ggdb3
endif

include $(ADK_ROOT)/make/desktop_ut_shared.mk

ifeq ($(ADK_APP_TYPE),lib)
# Library specific flags

CFLAGS += -fPIC -DPIC

LDFLAGS += -shared -fPIC

endif

################################################################################
# Executable binary

ifndef ADK_INSTALL_MODE
ADK_INSTALL_MODE = 0755
endif

ifeq ($(ADK_APP_TYPE),app)
BINARY = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)
ifndef ADK_INSTALL_DIR
ADK_INSTALL_DIR = $(ADK_PREFIX)/bin
endif
else ifeq ($(ADK_APP_TYPE),lib)
BINARY = $(ADK_OBJ_DIR)/lib$(ADK_APP_NAME).so
ifndef ADK_INSTALL_DIR
ADK_INSTALL_DIR = $(ADK_PREFIX)/lib
endif
endif

all: $(BINARY)

$(BINARY): $(ADK_OBJS)
	$(CC) $(LDFLAGS) $(LIB_FLAGS) -o $@ $^

$(ADK_OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<

define ADK_GCH_RECIPE
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -x c++-header -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -x c++-header -MM -MT '$@' -o $(@:.gch=.d) $<
endef

ifdef BINARY
install: $(BINARY)
	$(INSTALL) -d $(ADK_INSTALL_DIR)
	$(INSTALL) -m $(ADK_INSTALL_MODE) $(BINARY) $(ADK_INSTALL_DIR)
endif
