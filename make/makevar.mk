# /ADK/make/makevar.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

export DEF_GOAL = all
export TOOLS_BIN = $(ADK_PREFIX)/bin
export LIB_DIRS += $(ADK_PREFIX)/lib $(ADK_PREFIX)/lib64
export NAT_TOOLS_PREFIX = adk-
export AVR_TOOLS_PREFIX = avr-

# Common tools
export INSTALL = install
export DD = dd
export CP = cp
export RM = rm -f
export RMPATH = rm -rf
export CAT = cat
export MKDIR = mkdir
export MKPATH = mkdir -p
export RSYNC = rsync

# Default native platform
ifndef ADK_NAT_PLATFORM
export ADK_NAT_PLATFORM = linux64
endif

# Native compilation tools
export NAT_CC = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)gcc
export NAT_LD = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)ld
export NAT_NM = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)nm
export NAT_OBJCOPY = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)objcopy
export NAT_CPPFILT = $(TOOLS_BIN)/$(NAT_TOOLS_PREFIX)c++filt

ifeq ($(ADK_NAT_PLATFORM),linux32)
export LINUX32_CC = $(NAT_CC)
export LINUX32_LD = $(NAT_LD)
export LINUX32_NM = $(NAT_NM)
export LINUX32_OBJCOPY = $(NAT_OBJCOPY)
export LINUX32_CPPFILT = $(NAT_CPPFILT)
export LINUX64_CC = XXX
export LINUX64_LD = XXX
export LINUX64_NM = XXX
export LINUX64_OBJCOPY = XXX
export LINUX64_CPPFILT = XXX
else ifeq ($(ADK_NAT_PLATFORM),linux64)
export LINUX32_CC = XXX
export LINUX32_LD = XXX
export LINUX32_NM = XXX
export LINUX32_OBJCOPY = XXX
export LINUX32_CPPFILT = XXX
export LINUX64_CC = $(NAT_CC)
export LINUX64_LD = $(NAT_LD)
export LINUX64_NM = $(NAT_NM)
export LINUX64_OBJCOPY = $(NAT_OBJCOPY)
export LINUX64_CPPFILT = $(NAT_CPPFILT)
else
$(error Build platform not supported: $(ADK_NAT_PLATFORM))
endif

# AVR compilation tools
export AVR_CC = $(TOOLS_BIN)/$(AVR_TOOLS_PREFIX)gcc
export AVR_LD = $(TOOLS_BIN)/$(AVR_TOOLS_PREFIX)ld
export AVR_NM = $(TOOLS_BIN)/$(AVR_TOOLS_PREFIX)nm
export AVR_OBJCOPY = $(TOOLS_BIN)/$(AVR_TOOLS_PREFIX)objcopy
export AVR_CPPFILT = $(TOOLS_BIN)/$(AVR_TOOLS_PREFIX)c++filt

export WARN_COMP_FLAGS = -Wall -Werror
export STD_COMP_FLAGS = -std=c++11

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

export ADK_PLATFORM_ID_AVR = 0
export ADK_PLATFORM_ID_LINUX32 = 1
export ADK_PLATFORM_ID_LINUX64 = 2
export ADK_PLATFORM_ID_WIN32 = 3
export ADK_PLATFORM_ID_WIN64 = 4

# Desktop libraries
export DESKTOP_LIBS = \
	glib-$(GLIB_VERSION) \
    glibmm-$(GLIBMM_VERSION) \
    sigc-$(SIGCPP_VERSION)

# Desktop GUI libraries
export DESKTOP_GUI_LIBS = \
    gtkmm-$(GTKMM_VERSION) \
    atkmm-$(ATKMM_VERSION) \
    cairomm-$(CAIROMM_VERSION)

export LIB_INC_DIR = $(ADK_PREFIX)/include
export LIB_LIB_DIR = $(ADK_PREFIX)/lib

# Inlcude directories for desktop libraries
export DESKTOP_LIBS_INC_DIRS = \
	$(LIB_INC_DIR)/glib-$(GLIB_VERSION) \
    $(LIB_LIB_DIR)/glib-$(GLIB_VERSION)/include \
    $(LIB_INC_DIR)/glibmm-$(GLIBMM_VERSION) \
    $(LIB_LIB_DIR)/glibmm-$(GLIBMM_VERSION)/include \
    $(LIB_INC_DIR)/giomm-$(GIOMM_VERSION) \
    $(LIB_INC_DIR)/sigc++-$(SIGCPP_VERSION) \
    $(LIB_LIB_DIR)/sigc++-$(SIGCPP_VERSION)/include \

# Inlcude directories for desktop GUI libraries
export DESKTOP_GUI_LIBS_INC_DIRS = \
    $(LIB_INC_DIR)/freetype2 \
    $(LIB_INC_DIR)/pango-$(PANGO_VERSION) \
    $(LIB_INC_DIR)/cairo \
    $(LIB_INC_DIR)/gdk-pixbuf-$(GDK_PIXBUF_VERSION) \
    $(LIB_INC_DIR)/gtk-$(GTK_VERSION) \
    $(LIB_INC_DIR)/cairomm-$(CAIROMM_VERSION) \
    $(LIB_LIB_DIR)/cairomm-$(CAIROMM_VERSION)/include \
    $(LIB_INC_DIR)/pangomm-$(PANGOMM_VERSION) \
    $(LIB_LIB_DIR)/pangomm-$(PANGOMM_VERSION)/include \
    $(LIB_INC_DIR)/gdkmm-$(GDKMM_VERSION) \
    $(LIB_LIB_DIR)/gdkmm-$(GDKMM_VERSION)/include \
    $(LIB_INC_DIR)/gtkmm-$(GTKMM_VERSION) \
    $(LIB_LIB_DIR)/gtkmm-$(GTKMM_VERSION)/include \
    $(LIB_INC_DIR)/atk-$(ATK_VERSION) \
    $(LIB_INC_DIR)/atkmm-$(ATKMM_VERSION)
