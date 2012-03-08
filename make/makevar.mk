# /ADK/make/makevar.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

export DEF_GOAL = all

# Common tools
export INSTALL = install
export DD = dd
export CP = cp
export RM = rm -f
export RMPATH = rm -rf
export CAT = cat
export MKDIR = mkdir
export MKPATH = mkdir -p

# Native compilation tools
export NAT_CC = $(TOOLS_BIN)/gcc
export NAT_LD = $(TOOLS_BIN)/gcc
export NAT_NM = $(TOOLS_BIN)/nm
export NAT_CPPFILT = $(TOOLS_BIN)/c++filt

export WARN_CFLAGS = -Wall -Werror

