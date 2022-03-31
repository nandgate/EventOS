TARGET_NAME ?= EventOS
TARGET_ELF ?= $(TARGET_NAME).elf
TARGET_BIN ?= $(TARGET_NAME).bin
TARGET_LISTING ?= $(TARGET_NAME).lst

BUILD_DIR ?= ./build
SRC_DIRS ?= ./app

SRCS := $(shell find $(SRC_DIRS) -name *.c -or -name *.S)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
PREPROC_FLAGS := -MMD -MP
CPU_FLAGS := -D STM32F103xB -mcpu=cortex-m3 -Os
SPEC_FLAGS := -specs=nano.specs -specs=nosys.specs 

CC = arm-none-eabi-gcc
FLAGS ?= $(INC_FLAGS) $(PREPROC_FLAGS) $(CPU_FLAGS) $(SPEC_FLAGS) -g -std=c99 -fdata-sections -ffunction-sections -Wall
LDFLAGS := $(CPU_FLAGS) $(SPEC_FLAGS) -T gcc.ld -Wl,--gc-sections -Xlinker -Map=$(BUILD_DIR)/output.map -g

$(BUILD_DIR)/$(TARGET_ELF): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	arm-none-eabi-objdump -dSzw  $(BUILD_DIR)/$(TARGET_ELF) > $(BUILD_DIR)/$(TARGET_LISTING)
	arm-none-eabi-objcopy -O binary $@ $(BUILD_DIR)/$(TARGET_BIN)
	arm-none-eabi-size $@

# assembly
$(BUILD_DIR)/%.S.o: %.S
	$(MKDIR_P) $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(FLAGS) -c $< -o $@

.PHONY: clean docs

docs:
	$(RM) -r docs
	doxygen eventos.doxy

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
