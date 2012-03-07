# /ADK/make/makevar.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

# Common tools
export INSTALL = install
export DD = dd
export CP = cp
export RM = rm
export CAT = cat

# Native compilation tools
export NAT_CC = $(TOOLS_BIN)/gcc
export NAT_LD = $(TOOLS_BIN)/gcc
export NAT_NM = $(TOOLS_BIN)/nm
export NAT_CPPFILT = $(TOOLS_BIN)/c++filt
