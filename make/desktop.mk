# /ADK/make/desktop.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

LIBS += stdc++ $(DESKTOP_LIBS)

INCLUDE_DIRS += $(DESKTOP_LIBS_INC_DIRS)

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
	echo "extern \"C\" const char _binary_$(patsubst %.glade,%,$<)_glade_start;" >> $@
	echo "extern \"C\" const char _binary_$(patsubst %.glade,%,$<)_glade_end;" >> $@

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

endif

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
