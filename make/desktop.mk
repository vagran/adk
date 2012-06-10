# /ADK/make/desktop.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

LIBS += stdc++ glib-$(GLIB_VERSION) glibmm-$(GLIBMM_VERSION) \
	sigc-$(SIGCPP_VERSION) gtkmm-$(GTKMM_VERSION) atkmm-$(ATKMM_VERSION) \
	cairomm-$(CAIROMM_VERSION)

LIB_INC_DIR += $(ADK_PREFIX)/include
LIB_DIRS += $(ADK_PREFIX)/lib

INCLUDE_DIRS += $(LIB_INC_DIR)/freetype2

INCLUDE_DIRS += $(LIB_INC_DIR)/glib-$(GLIB_VERSION) \
                $(LIB_INC_DIR)/pango-$(PANGO_VERSION) \
                $(LIB_INC_DIR)/cairo \
                $(LIB_INC_DIR)/gdk-pixbuf-$(GDK_PIXBUF_VERSION) \
                $(LIB_INC_DIR)/gtk-$(GTK_VERSION) \
                $(LIB_INC_DIR)/sigc++-$(SIGCPP_VERSION) \
                $(LIB_INC_DIR)/glibmm-$(GLIBMM_VERSION) \
                $(LIB_INC_DIR)/giomm-$(GIOMM_VERSION) \
                $(LIB_INC_DIR)/cairomm-$(CAIROMM_VERSION) \
                $(LIB_INC_DIR)/pangomm-$(PANGOMM_VERSION) \
                $(LIB_INC_DIR)/gdkmm-$(GDKMM_VERSION) \
                $(LIB_INC_DIR)/gtkmm-$(GTKMM_VERSION) \
                $(LIB_INC_DIR)/atk-$(ATK_VERSION) \
                $(LIB_INC_DIR)/atkmm-$(ATKMM_VERSION)

ifeq ($(ADK_BUILD_TYPE),release)
	CFLAGS += -O2
else ifeq ($(ADK_BUILD_TYPE),debug)
    CFLAGS += -O0 -ggdb3
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
	$(CC) $(LDFLAGS) -o $@ $^ $(LIB_FLAGS)

$(ADK_OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<
