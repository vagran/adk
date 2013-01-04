# /ADK/make/avr.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

################################################################################
# Compilation parameters

ifndef ADK_MCU
    $(error Target MCU model is not defined)
endif

DEFS += ADK_MCU=$(ADK_MCU)

INCLUDE_DIRS += $(ADK_PREFIX)/avr/include
LIB_DIRS += $(ADK_PREFIX)/avr/lib

ifeq ($(ADK_BUILD_TYPE),release)
    ifndef RELEASE_OPT_FLAGS
        # Optimize for size by default
        COMMON_COMP_FLAGS += -Os
    else
        COMMON_COMP_FLAGS += $(RELEASE_OPT_FLAGS)
    endif
else ifeq ($(ADK_BUILD_TYPE),debug)
    ifndef DEBUG_OPT_FLAGS
        # Do not optimize by default
        COMMON_COMP_FLAGS += -O0
    else
        COMMON_COMP_FLAGS += $(DEBUG_OPT_FLAGS)
    endif
else
    $(error Build type not supported: $(ADK_BUILD_TYPE))
endif

COMMON_COMP_FLAGS += -mmcu=$(ADK_MCU)

################################################################################
# Executable binary

BINARY = $(ADK_OBJ_DIR)/$(ADK_APP_NAME).elf

all: $(BINARY)

$(BINARY): $(ADK_OBJS)
	$(CC) $(LDFLAGS) $(LIB_FLAGS) -o $@ $^ $(LIBS)

$(ADK_OBJ_DIR)/%.o: %.c
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<

define ADK_GCH_RECIPE
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -x c-header -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(CFLAGS) -x c-header -MM -MT '$@' -o $(@:.gch=.d) $<
endef

#XXX cpp S

################################################################################
# Binary converted to text formats understandable by most firmware uploaders.

AVR_ROM_HEX = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)_rom.hex
AVR_ROM_SREC = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)_rom.srec
AVR_ROM_BIN = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)_rom.bin

.PHONY: avr_rom_text

all: avr_rom_text

avr_rom_text: $(AVR_ROM_HEX) $(AVR_ROM_SREC) $(AVR_ROM_BIN)

$(AVR_ROM_HEX): $(BINARY)
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(AVR_ROM_SREC): $(BINARY)
	$(OBJCOPY) -j .text -j .data -O srec $< $@
	
$(AVR_ROM_BIN): $(BINARY)
	$(OBJCOPY) -j .text -j .data -O binary $< $@

################################################################################
# EEPROM images

AVR_EEPROM_HEX = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)_eeprom.hex
AVR_EEPROM_SREC = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)_eeprom.srec
AVR_EEPROM_BIN = $(ADK_OBJ_DIR)/$(ADK_APP_NAME)_eeprom.bin

.PHONY: avr_eeprom_text

all: avr_eeprom_text

avr_eeprom_text: $(AVR_EEPROM_HEX) $(AVR_EEPROM_SREC) $(AVR_EEPROM_BIN)

$(AVR_EEPROM_HEX): $(BINARY)
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

$(AVR_EEPROM_SREC): $(BINARY)
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@
	
$(AVR_EEPROM_BIN): $(BINARY)
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@
