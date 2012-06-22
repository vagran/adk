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
$(ADK_OBJ_DIR)/%.glade.h: %.glade
	echo "/* This file is generated automatically from \"$<\" file. */" > $@
	echo "extern \"C\" const char _binary_$(patsubst %.glade,%,$<)_glade_start;" >> $@
	echo "extern \"C\" const char _binary_$(patsubst %.glade,%,$<)_glade_end;" >> $@

$(ADK_OBJS): $(GLADE_AUTO_HDR)

# Header file which includes all automatic glade header files
$(GLADE_AUTO_HDR): $(GLADE_HDR_FILES)
	echo "/* This file is generated automatically. */" > $@
	$(foreach file, $(GLADE_HDR_FILES), echo "#include <$(notdir $(file))>" >> $@)

# Object file from Glade XML
$(ADK_OBJ_DIR)/%.glade.o: %.glade
	$(OBJCOPY) -I binary -O $(OBJ_FORMAT) -B $(OBJ_ARCH) $< $@

endif

ifeq ($(ADK_APP_TYPE),lib)
# Library specific flags

CFLAGS += -fPIC -DPIC

LDFLAGS += -shared -fPIC

endif

################################################################################
# Executable binary

ifeq ($(ADK_APP_TYPE),app)
BINARY = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)
else ifeq ($(ADK_APP_TYPE),lib)
BINARY = $(ADK_OBJ_DIR)/lib$(ADK_APP_NAME).so
endif

all: $(BINARY)

$(BINARY): $(ADK_OBJS)
	$(CC) $(LDFLAGS) $(LIB_FLAGS) -o $@ $^

$(ADK_OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<
