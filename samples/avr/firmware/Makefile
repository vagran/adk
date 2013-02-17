# /ADK/samples/avr/firmware/Makefile
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
# MCU clock frequency in Hz
ADK_MCU_FREQ = 20000000

# L 0b11011111 H 0b11011111- 20MHz, BOD 2.7V
ADK_MCU_LFUSE = 0xdf
ADK_MCU_HFUSE = 0xdb

# Make USB device
ADK_AVR_USE_USB = yes

# Additional directory with source files
SRC_DIRS += pwm
# Additional source file
SRCS += misc/misc.c
# Additional C compilation flags
CFLAGS += -Davr_sample_1
# Additional include directories
INCLUDE_DIRS += . ../include misc
# Additional defines
DEFS += AVR_USB_DEBUG

include $(ADK_ROOT)/make/make.mk
