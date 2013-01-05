# /ADK/samples/avr/Makefile
#
# This file is a part of ADK library.
# Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

ADK_PLATFORM = avr
ADK_APP_TYPE = app
ADK_APP_NAME = avr_sample_1

# MCU code name
ADK_MCU = attiny2313

# Programmer type
ADK_PROGRAMMER = avrisp2
# Programmer connection
ADK_PROGRAMMER_BUS = /dev/ttyUSB0

# 0b1111101 - 4MHz slow start
ADK_MCU_LFUSE = 0xfd

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
