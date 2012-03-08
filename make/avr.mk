# /ADK/make/avr.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

################################################################################
# Executable binary

BINARY = $(ADK_APP_NAME).elf

all: $(BINARY)

$(BINARY): $(OBJS)

# XXX

################################################################################
# Binary converted to text formats understandable by most firmware uploaders.

.PHONY: text

# XXX
