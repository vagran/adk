# /ADK/make/makevar.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

export DEF_GOAL = all
export TOOLS_BIN = $(ADK_PREFIX)/bin
export LIB_DIRS = $(ADK_PREFIX)/lib $(ADK_PREFIX)/lib64
export NAT_TOOLS_PREFIX = adk-
export NAT_PLATFORM = linux64

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
export NAT_CC = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)gcc
export NAT_LD = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)ld
export NAT_NM = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)nm
export NAT_OBJCOPY = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)objcopy
export NAT_CPPFILT = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)c++filt

export WARN_COMP_FLAGS = -Wall -Werror
export STD_COMP_FLAGS = -std=c++0x

# Used libraries versions
export GLIB_VERSION = 2.0
export PANGO_VERSION = 1.0
export GDK_PIXBUF_VERSION = 2.0
export GTK_VERSION = 3.0
export GLIBMM_VERSION = 2.4
export GIOMM_VERSION = 2.4
export CAIROMM_VERSION = 1.0
export PANGOMM_VERSION = 1.4
export GDKMM_VERSION = 3.0
export GTKMM_VERSION = 3.0
export SIGCPP_VERSION = 2.0
export ATK_VERSION = 1.0
export ATKMM_VERSION = 1.6
