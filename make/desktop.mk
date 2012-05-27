# /ADK/make/desktop.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

LIBS += stdc++

LIBS := $(foreach lib, $(LIBS), -l$(lib))

################################################################################
# Executable binary

BINARY = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)

all: $(BINARY)

$(BINARY): $(ADK_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(ADK_OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<
