TARGET_NAME ?= Morse-posix
BUILD_DIR   ?= ./build

EVENTOS_ROOT := ../..
POSIX_DIR    := ../posix

CTX_ALLOC ?= ctxAllocMalloc

OS_SRCS := $(filter-out $(wildcard $(EVENTOS_ROOT)/app/source/os/mem/ctxAlloc*.c), \
             $(shell find $(EVENTOS_ROOT)/app/source/os -name '*.c'))
OS_SRCS += $(EVENTOS_ROOT)/app/source/os/mem/$(CTX_ALLOC).c

PLATFORM_SRCS := $(shell find $(POSIX_DIR)/source -name '*.c')

APP_SRCS := main.c

SRCS := $(OS_SRCS) $(PLATFORM_SRCS) $(APP_SRCS)

strip_dotdot = $(subst ../,,$(1))
OBJS := $(patsubst %,$(BUILD_DIR)/%.o,$(call strip_dotdot,$(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_FLAGS := -I$(EVENTOS_ROOT)/app/include -I$(POSIX_DIR)/include

CC = cc
CFLAGS := $(INC_FLAGS) -MMD -MP -O2 -g -std=c99 -Wall
LDFLAGS := -g

$(BUILD_DIR)/$(TARGET_NAME): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/app/%.c.o: $(EVENTOS_ROOT)/app/%.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/posix/%.c.o: $(POSIX_DIR)/%.c
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
