TARGET_NAME ?= Blinky
TARGET_ELF ?= $(TARGET_NAME).elf
TARGET_BIN ?= $(TARGET_NAME).bin
TARGET_LISTING ?= $(TARGET_NAME).lst

BUILD_DIR ?= ./build

# EventOS root (relative to this Makefile)
EVENTOS_ROOT := ../..
BLUEPILL_DIR := ../bluepill

# Context allocator selection
CTX_ALLOC ?= ctxAllocMalloc

# Source files
OS_SRCS := $(filter-out $(wildcard $(EVENTOS_ROOT)/app/source/os/mem/ctxAlloc*.c), \
             $(shell find $(EVENTOS_ROOT)/app/source/os -name '*.c'))
OS_SRCS += $(EVENTOS_ROOT)/app/source/os/mem/$(CTX_ALLOC).c

PLATFORM_SRCS := $(shell find $(BLUEPILL_DIR)/source -name '*.c' -or -name '*.S')

APP_SRCS := main.c

SRCS := $(OS_SRCS) $(PLATFORM_SRCS) $(APP_SRCS)

# Strip ../ prefixes so object files stay inside BUILD_DIR
strip_dotdot = $(subst ../,,$(1))
OBJS := $(patsubst %,$(BUILD_DIR)/%.o,$(call strip_dotdot,$(SRCS)))
DEPS := $(OBJS:.o=.d)

# Include paths
INC_FLAGS := -I$(EVENTOS_ROOT)/app/include \
             -I$(BLUEPILL_DIR)/include \
             -I$(BLUEPILL_DIR)/cmsis/Include \
             -I$(BLUEPILL_DIR)/stm32f1xx/Include

# Compiler settings
CC = arm-none-eabi-gcc
CPU_FLAGS := -D STM32F103xB -mcpu=cortex-m3 -Os
SPEC_FLAGS := -specs=nano.specs -specs=nosys.specs
PREPROC_FLAGS := -MMD -MP
CFLAGS := $(INC_FLAGS) $(PREPROC_FLAGS) $(CPU_FLAGS) $(SPEC_FLAGS) -g -std=c99 -fdata-sections -ffunction-sections -Wall
LDFLAGS := $(CPU_FLAGS) $(SPEC_FLAGS) -T $(BLUEPILL_DIR)/gcc.ld -Wl,--gc-sections -Xlinker -Map=$(BUILD_DIR)/output.map -g

$(BUILD_DIR)/$(TARGET_ELF): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	arm-none-eabi-objdump -dSzw $@ > $(BUILD_DIR)/$(TARGET_LISTING)
	arm-none-eabi-objcopy -O binary $@ $(BUILD_DIR)/$(TARGET_BIN)
	arm-none-eabi-size $@

# Pattern rules: map stripped object paths back to actual source locations
$(BUILD_DIR)/app/%.c.o: $(EVENTOS_ROOT)/app/%.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/bluepill/%.S.o: $(BLUEPILL_DIR)/%.S
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/bluepill/%.c.o: $(BLUEPILL_DIR)/%.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
