# /ADK/make/desktop_ut_shared.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

# Definitions common for both desktop and unit tests targets (unit tests are
# assumed to be hosted on desktop platform).

LIBS += stdc++ $(DESKTOP_LIBS)

INCLUDE_DIRS += $(DESKTOP_LIBS_INC_DIRS)

ifeq ($(ADK_USE_GUI),yes)

# Get glade files
GLADE_FILES += $(foreach src_dir, $(SRC_DIRS), $(call SCAN_PATH, $(src_dir), *.glade))

ifneq ($(GLADE_FILES), )

# Create object files from Glade XML files
GLADE_OBJ_FILES = $(foreach file, $(GLADE_FILES), $(ADK_OBJ_DIR)/$(notdir $(file:.glade=.glade.o)))
GLADE_HDR_FILES = $(foreach file, $(GLADE_FILES), $(ADK_OBJ_DIR)/$(notdir $(file:.glade=.glade.h)))
GLADE_AUTO_HDR = $(ADK_OBJ_DIR)/auto_adk_glade.h
ADK_OBJS += $(GLADE_OBJ_FILES)

# Header file with definition of XML location in data section
$(ADK_OBJ_DIR)/%.glade.h: %.glade $(ADK_BUILD_DIR)
	echo "/* This file is generated automatically from \"$<\" file. */" > $@
	echo "extern \"C\" const char _binary_$(notdir $(patsubst %.glade,%,$<))_glade_start;" >> $@
	echo "extern \"C\" const char _binary_$(notdir $(patsubst %.glade,%,$<))_glade_end;" >> $@

$(ADK_OBJS): $(GLADE_AUTO_HDR)

# Main ADK include file should depend on Glade automatic header in order to
# make pre-compiled header.
$(ADK_ROOT)/include/adk.h: $(GLADE_AUTO_HDR)

# Header file which includes all automatic glade header files
$(GLADE_AUTO_HDR): $(GLADE_HDR_FILES) $(ADK_BUILD_DIR)
	echo "/* This file is generated automatically. */" > $@
	$(foreach file, $(GLADE_HDR_FILES), echo "#include <$(notdir $(file))>" >> $@)

# Object file from Glade XML
$(ADK_OBJ_DIR)/%.glade.o: %.glade $(ADK_BUILD_DIR)
	$(OBJCOPY) -I binary -O $(OBJ_FORMAT) -B $(OBJ_ARCH) $< $@

# GLADE_FILES
endif

# ADK_USE_GUI
endif
