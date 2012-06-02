# /ADK/make/desktop.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

LIBS += stdc++ glibmm-$(GLIBMM_VERSION) gtkmm-$(GTKMM_VERSION)

LIB_FLAGS = $(foreach lib, $(LIBS), -l$(lib))

LIB_INC_DIR = $(ADK_PREFIX)/include
LIB_LIB_DIR = $(ADK_PREFIX)/lib

LIB_FLAGS += -L$(LIB_LIB_DIR)

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
GLADE_TARGET_FILES = $(foreach file, $(GLADE_FILES), $(ADK_OBJ_DIR)/$(notdir $(file)))
# Rules for glade files
define GLADE_RULE
$(ADK_OBJ_DIR)/$(notdir $(1)): $(1)
	cp $$^ $$@
endef
$(foreach file, $(GLADE_FILES), $(eval $(call GLADE_RULE, $(file))))
all: $(GLADE_TARGET_FILES)
endif

################################################################################
# Executable binary

BINARY = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)

all: $(BINARY)

$(BINARY): $(ADK_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIB_FLAGS)

$(ADK_OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<
