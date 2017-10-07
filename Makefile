PROJECT = test

##############################################################
##############################################################
##############################################################
EXECUTABLE = $(PROJECT).elf
BIN_IMAGE = $(PROJECT).bin
HEX_IMAGE = $(PROJECT).hex

# Toolchain configurations
#CROSS_COMPILE = arm-none-eabi-
CROSS_COMPILE = /home/alex/gcc-arm-none-eabi-5_4-2016q2/bin/arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

# Cortex-M4 implements the ARMv7E-M architecture
CPU = cortex-m4
CFLAGS += -mcpu=$(CPU) -march=armv7e-m -mtune=cortex-m4
CFLAGS += -mlittle-endian -mthumb

# Basic configurations
CFLAGS += -g -std=c99
CFLAGS += -Wall

# Optimizations
CFLAGS += -O0

# PIC
CFLAGS += -fPIC
CFLAGS += -mno-pic-data-is-text-relative
#CFLAGS += -msingle-pic-base
#CFLAGS += -mpic-register=r10

CFLAGS += -DSTM32F429_439xx
CFLAGS += -DVECT_TAB_FLASH
CFLAGS += -DUSE_STDPERIPH_DRIVER

LDFLAGS += -mcpu=cortex-m3
LDFLAGS += -mlittle-endian
LDFLAGS += -mthumb
LDFLAGS += -Wl,--gc-section

LDFLAGS += $(LDSCRIPT) --specs=nano.specs --specs=nosys.specs

##############################################################
##############################################################
##############################################################

# set the path to STM32F429I-Discovery firmware package
STDP = ./STM32F429I-Discovery_FW_V1.0.1


LDSCRIPT = -T./STM32F429ZI_FLASH.ld


INC += -I$(STDP)/Libraries/CMSIS/Device/ST/STM32F4xx/Include
INC += -I$(STDP)/Libraries/CMSIS/Include
INC += -I$(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/inc
INC += -I$(STDP)/Utilities/STM32F429I-Discovery
INC += -I./User/inc


ASRC += $(STDP)/Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f429_439xx.s


CSRC += $(STDP)/Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

CSRC += \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c \
    $(STDP)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c

CSRS += $(STDP)/Utilities/STM32F429I-Discovery/stm32f429i_discovery.c
CSRC += $(STDP)/Utilities/STM32F429I-Discovery/stm32f429i_discovery_lcd.c
CSRC += $(STDP)/Utilities/STM32F429I-Discovery/stm32f429i_discovery_sdram.c

CSRC += ./User/src/main.c
CSRC += ./User/src/stm32f4xx_it.c

##############################################################
##############################################################
##############################################################

OBJ += $(CSRC:%.c=%.o)
OBJ += $(ASRC:%.s=%.o)

all: $(BIN_IMAGE)

$(BIN_IMAGE): $(EXECUTABLE)
	$(OBJCOPY) -O binary $^ $@
	$(OBJCOPY) -O ihex $^ $(HEX_IMAGE)
	$(OBJDUMP) -h -S -D $(EXECUTABLE) > $(PROJECT).lst
	$(SIZE) $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLE)
	rm -rf $(BIN_IMAGE)
	rm -rf $(HEX_IMAGE)
	rm -f $(OBJ)
	rm -f $(PROJECT).lst

flash:
	openocd -f interface/stlink-v2.cfg \
		-f target/stm32f4x_stlink.cfg \
		-c "init" \
		-c "reset init" \
		-c "halt" \
		-c "flash write_image erase $(PROJECT).elf" \
		-c "verify_image $(PROJECT).elf" \
    -c "reset run" -c shutdown

#flash:
#	st-flash write $(BIN_IMAGE) 0x8000000

.PHONY: clean
