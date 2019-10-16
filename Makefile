PROJ_NAME := main
OUTPUT_DIR := output
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
DEP_DIR := $(BUILD_DIR)/dep
LDSCRIPT := stm32_flash.ld
COMPILER_CONF := ./src/include/compiler_config.h

###################################################

CC := @arm-none-eabi-gcc
LD := @arm-none-eabi-gcc
AS := @arm-none-eabi-as
OBJCOPY := @arm-none-eabi-objcopy

###################################################

CFLAGS := -O2 -Wall -T$(LDSCRIPT) -include $(COMPILER_CONF)
#CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -std=gnu99
#CFLAGS += -ffreestanding -nostdlib
#CFLAGS += --specs=nosys.specs

LDFLAGS := -g -O0 -Wall -T$(LDSCRIPT)
#LDFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
LDFLAGS += -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDFLAGS += -specs=nosys.specs
LDFLAGS += -u _printf_float --enable-newlib-io-long-long

DFLAGS := -MD -MP -MF $(DEP_DIR)/$*.d
CFLAGS += $(DFLAGS)

CFLAGS += $(addprefix -D, $(DEFINES))

###################################################

ROOT := $(shell pwd)

SRCS := $(shell find . -name '*.c')
SRCS += $(shell find . -name '*.s')

INCLUDE_DIR_NAMES = src include inc Inc Include drivers
INCLUDE_DIRS = $(foreach dir, $(INCLUDE_DIR_NAMES), $(shell find -type d -name $(dir)))
INCLUDE_DIRS := $(filter-out %$(BUILD_DIR), $(INCLUDE_DIRS)) # don't include files in the build directory
CFLAGS += $(addprefix -I, $(INCLUDE_DIRS))

OBJS := $(SRCS:./%.c=$(OBJ_DIR)/%.o)
OBJS := $(OBJS:./%.s=$(OBJ_DIR)/%.o)

DEPS := $(SRCS:./%.c=$(DEP_DIR)/%.d)
DEPS := $(DEPS:%.s=$(DEP_DIR)/%.d)

$(foreach dep, $(DEPS), $(shell mkdir -p $(shell dirname $(dep)) >/dev/null))

#$(info SRCS = $(SRCS))
#$(info INCLUDE_DIRS = $(INCLUDE_DIRS))
#$(info OBJS = $(OBJS))
#$(info DEPS = $(DEPS))

###################################################

.PHONY: all install clean

all: $(OUTPUT_DIR)/$(PROJ_NAME).bin

$(OBJ_DIR)/%.o: %.s
	$(info AS $^)
	$(AS) -c -o $@ $^

$(OBJ_DIR)/%.o: %.c
	$(info CC $^)
	$(shell mkdir -p $(shell dirname $@) >/dev/null)
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUTPUT_DIR)/$(PROJ_NAME).bin: $(OBJS)
	$(info LD $^)
	$(shell mkdir -p $(OUTPUT_DIR) >/dev/null)
	$(LD) $(LDFLAGS) $^ -o $(OUTPUT_DIR)/$(PROJ_NAME).elf
	$(OBJCOPY) -O binary $(OUTPUT_DIR)/$(PROJ_NAME).elf $(OUTPUT_DIR)/$(PROJ_NAME).bin

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(OUTPUT_DIR)

install: all
	st-flash write $(OUTPUT_DIR)/$(PROJ_NAME).bin 0x8000000

-include $(DEPS)
