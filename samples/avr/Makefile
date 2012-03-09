# /ADK/samples/avr/Makefile
#
# This file is a part of ADK library.
# Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

ADK_PLATFORM = avr
ADK_APP_TYPE = app
ADK_APP_NAME = avr_sample_1
ADK_MCU = attiny2313

# Additional directory with source files
SRC_DIRS = pwm
# Additional source file
SRCS = misc/misc.c
# Additional C compilation flags
CFLAGS = -Davr_sample_1
# Additional include directories
INCLUDE_DIRS = misc
# Additional defines
DEFS = AVR_SAMPLE_1 REV=0x0100

include $(ADK_ROOT)/make/make.mk
