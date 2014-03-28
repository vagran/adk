# /ADK/make/desktop_ut_shared.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

# Definitions common for both desktop and unit tests targets (unit tests are
# assumed to be hosted on desktop platform).

ifndef ADK_NO_ADK_LIB
LIBS += adk
endif

LIBS += stdc++ $(DESKTOP_LIBS) expat

INCLUDE_DIRS += $(DESKTOP_LIBS_INC_DIRS)


# Embedded .glade XML files support 

ifeq ($(ADK_USE_GUI),yes)

# Get glade files
RES_FILES += $(foreach src_dir, $(SRC_DIRS), $(call SCAN_PATH, $(src_dir), *.glade))

# ADK_USE_GUI
endif
