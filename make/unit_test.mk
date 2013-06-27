# /ADK/make/unit_test.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

# Directory with unit test framework source files
UT_DIR = $(ADK_ROOT)/src/unit_test
# Tool for automatic stubs generation
STUBS_GEN = $(UT_DIR)/ut_stubs_gen.py

BINARY_NAME = $(ADK_OBJ_DIR)/$(ADK_TEST_NAME)

SRC_DIRS += $(UT_DIR)
VPATH += $(dir $(ADK_TEST_SRCS))

DEFS += UNITTEST

CFLAGS += -ggdb3 -DDEBUG -O0

LIBS += c adk

# Directories with detached libraries debug symbols
ifeq ($(ADK_PLATFORM),linux64)
    DBG_LIB_DIRS += /usr/lib/debug/lib/x86_64-linux-gnu
endif

include $(ADK_ROOT)/make/desktop_ut_shared.mk

AUTO_SRC = $(ADK_OBJ_DIR)/auto_stabs.cpp
AUTO_OBJ = $(AUTO_SRC:.cpp=.o)

ADK_TEST_OBJS = $(foreach src, $(notdir $(ADK_TEST_SRCS)), $(ADK_OBJ_DIR)/$(src:.cpp=.o))
ADK_TEST_DEPS += $(ADK_TEST_OBJS:.o=.d)
# include dependencies if exist
-include $(ADK_TEST_DEPS)

define AUTO_CHUNK

namespace ut {

const char *__ut_test_description = "$(ADK_TEST_DESC)";

} /* namespace ut */

endef

VALGRIND = valgrind -q --suppressions=$(ADK_ROOT)/tools/valgrind.supp \
	--error-exitcode=255 --leak-check=full --gen-suppressions=all

################################################################################

all: $(BINARY_NAME)

$(ADK_TEST_OBJS) $(AUTO_OBJ): $(ADK_PCHS) $(ADK_BUILD_DIR)

$(BINARY_NAME): $(ADK_OBJS) $$(ADK_TEST_OBJS) $$(AUTO_OBJ)
	$(CC) $(LIB_FLAGS) $^ -o $@

define ADK_GCH_RECIPE
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -x c++-header -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -x c++-header -MM -MT '$@' -o $(@:.gch=.d) $<
endef

$(ADK_OBJ_DIR)/%.o: %.cpp
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<

$(AUTO_OBJ): $$(AUTO_SRC) $(ADK_BUILD_DIR)
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_CPP_FLAGS) $(CFLAGS) -o $@ $<

export AUTO_CHUNK
$(AUTO_SRC): $$(ADK_OBJS) $$(ADK_TEST_OBJS)
	$(STUBS_GEN) --nm $(NM) --cppfilt $(CPPFILT) \
	--result $@ \
	$(foreach src, $(ADK_TEST_OBJS), --test-src $(src)) \
	$(foreach src, $(ADK_OBJS), --src $(src)) \
	$(foreach dir, $(DBG_LIB_DIRS), --lib-dir $(dir)) \
	$(foreach dir, $(LIB_DIRS), --lib-dir $(dir)) \
	$(foreach lib, $(LIBS), --lib $(lib))
	echo "$$AUTO_CHUNK" >> $@

# Target for launching the tests
test: $(BINARY_NAME)
	(export LD_LIBRARY_PATH=$(call JOIN,:,$(LIB_DIRS)) && export PATH=$(TOOLS_BIN) && $(VALGRIND) $(BINARY_NAME))
