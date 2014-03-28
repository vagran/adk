# /ADK/unittest/python/Makefile
#
# This file is a part of ADK library.
# Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

ADK_APP_TYPE = unit_test

ADK_TEST_NAME = python
ADK_TEST_DESC = Embedded Python functionality

ADK_USE_PYTHON = yes
RES_FILES += test_embedding.py test_extending.py

ADK_TEST_SRCS = $(wildcard $(ADK_ROOT)/src/lib/*.cpp)

include $(ADK_ROOT)/make/make.mk
