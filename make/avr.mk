# /ADK/make/avr.mk
#
# This file is a part of ADK library.
# Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
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
        COMMON_COMP_FLAGS += -Os -mcall-prologues
    else
        COMMON_COMP_FLAGS += $(RELEASE_OPT_FLAGS)
    endif
else ifeq ($(ADK_BUILD_TYPE),debug)
    ifndef DEBUG_OPT_FLAGS
        # Optimize for size by default
        COMMON_COMP_FLAGS += -Os -mcall-prologues
    else
        COMMON_COMP_FLAGS += $(DEBUG_OPT_FLAGS)
    endif
    COMMON_COMP_FLAGS += -ggdb3
else
    $(error Build type not supported: $(ADK_BUILD_TYPE))
endif

COMMON_COMP_FLAGS += -mmcu=$(ADK_MCU) -fshort-wchar
LDFLAGS += -mmcu=$(ADK_MCU)

ifndef ADK_MCU_FREQ
$(error ADK_MCU_FREQ variable should be defined)
# ADK_MCU_FREQ
endif

DEFS += ADK_MCU_FREQ=$(ADK_MCU_FREQ)

ifeq ($(ADK_AVR_USE_USB),yes)

DEFS += ADK_AVR_USE_USB
SRC_DIRS += $(ADK_ROOT)/src/libavr/usb

# ADK_AVR_USE_USB
endif

################################################################################
# Executable binary

BINARY = $(ADK_OBJ_DIR)/$(ADK_APP_NAME).elf

all: $(BINARY)

$(BINARY): $$(ADK_OBJS)
	$(CC) $(LDFLAGS) $(LIB_FLAGS) -o $@ $^ $(LIBS)
	@echo
	@echo =========================== Image size ===========================
	@$(SIZE) $@
	@echo ==================================================================
	@echo

$(ADK_OBJ_DIR)/%.o: %.c
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_C_FLAGS) $(CFLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) $(COMMON_C_FLAGS) $(CFLAGS) -MM -MT '$@' -o $(@:.o=.d) $<
	
$(ADK_OBJ_DIR)/%.o: %.S
	$(CC) -c $(COMMON_COMP_FLAGS) -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) -MM -MT '$@' -o $(@:.o=.d) $<

define ADK_GCH_RECIPE
	$(CC) -c $(COMMON_COMP_FLAGS) -x c-header -o $@ $<
	$(CC) -c $(COMMON_COMP_FLAGS) -x c-header -MM -MT '$@' -o $(@:.gch=.d) $<
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
	
################################################################################
# Fuse bits programming

# Fuse byte for device with single fuse byte
ifdef ADK_MCU_FUSE
FUSE_CMD = $(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U fuse:w:$(ADK_MCU_FUSE):m
DEFS += ADK_MCU_FUSE=$(ADK_MCU_FUSE)
endif

# Fuse low byte
ifdef ADK_MCU_LFUSE
LFUSE_CMD = $(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U lfuse:w:$(ADK_MCU_LFUSE):m
DEFS += ADK_MCU_LFUSE=$(ADK_MCU_LFUSE)
endif

# Fuse high byte
ifdef ADK_MCU_HFUSE
HFUSE_CMD = $(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U hfuse:w:$(ADK_MCU_HFUSE):m
DEFS += ADK_MCU_HFUSE=$(ADK_MCU_HFUSE)
endif

# Fuse extended byte
ifdef ADK_MCU_EFUSE
EFUSE_CMD = $(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U efuse:w:$(ADK_MCU_EFUSE):m
DEFS += ADK_MCU_EFUSE=$(ADK_MCU_EFUSE)
endif

.PHONY: fuse upload_flash upload_eeprom upload verify_flash verify_eeprom verify reset

# Program fuse bytes
fuse:
	$(FUSE_CMD)
	$(LFUSE_CMD)
	$(HFUSE_CMD)
	$(EFUSE_CMD)

# Upload program flash only without EEPROM
upload_flash: $(AVR_ROM_HEX)
	$(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U flash:w:$(AVR_ROM_HEX):i
	
upload_eeprom: $(AVR_EEPROM_HEX)
	$(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U eeprom:w:$(AVR_EEPROM_HEX):i

# Upload firmware (program flash and data EEPROM)
upload: upload_flash upload_eeprom

verify_flash: $(AVR_ROM_HEX)
	$(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U flash:v:$(AVR_ROM_HEX):i
	
verify_eeprom: $(AVR_EEPROM_HEX)
	$(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS) -U eeprom:v:$(AVR_EEPROM_HEX):i

# Verify firmware (program flash and data EEPROM)
verify: verify_flash verify_eeprom

# Just check connection and also reset CPU.
reset:
	$(AVRDUDE) -p $(ADK_MCU) -c $(ADK_PROGRAMMER) -P $(ADK_PROGRAMMER_BUS)
