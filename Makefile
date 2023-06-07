## Cross-compilation commands 
CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-gcc
AR      = arm-none-eabi-ar
AS      = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE    = arm-none-eabi-size

TARGET = stm32g030xx
TARGET_UPPER = STM32G030xx
LINKER_SCRIPT = link_$(TARGET).ld

# our code
OBJS  = main.o
# startup files and anything else
OBJS += lib/cmsis_device_g0/Source/Templates/system_stm32g0xx.o lib/cmsis_device_g0/Source/Templates/gcc/startup_$(TARGET).o

## Platform and optimization options
CFLAGS += -c -fno-common -Os -ggdb3 -mcpu=cortex-m0 -mthumb -D$(TARGET_UPPER)
CFLAGS += -Wall -ffunction-sections -fdata-sections
CFLAGS += -Wno-unused-function -ffreestanding
LFLAGS  = -T$(LINKER_SCRIPT) -T common.ld --specs=nano.specs -Wl,--gc-sections -ggdb3 -Wl,--print-memory-usage -Wl,-Map=$@.map

## Locate the main libraries
CMSIS     = lib/cmsis_5/CMSIS/Core
CMSIS_SRC = lib/cmsis_device_g0/Source/Templates

## Drivers/library includes
INC     = -I$(CMSIS)/Include/
INC    += -Ilib/cmsis_device_g0/Include/ -I./
CFLAGS += $(INC)

CORE_OBJ_SRC         = $(wildcard $(HAL_SRC)/*.c)
CORE_OBJ_SRC        += $(wildcard $(CMSIS_SRC)/*.c)

CORE_LIB_OBJS        = $(CORE_OBJ_SRC:.c=.o)
CORE_LOCAL_LIB_OBJS  = $(notdir $(CORE_LIB_OBJS))

## Rules
all: size flash

main.elf: $(OBJS) $(LINKER_SCRIPT) common.ld
	$(LD) $(LFLAGS) -mcpu=cortex-m0 -mthumb -o $@ $(OBJS)

%.bin: %.elf
	$(OBJCOPY) --strip-unneeded -O binary $< $@

clean:
	rm main.elf main.o
