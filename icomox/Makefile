CMSIS_DIR     = CMSIS
VENDOR_DIR    = ADuCM4x50
FREE_RTOS_DIR = FreeRTOS
FATFS_DIR     = FatFS

CMSIS_INC     = $(CMSIS_DIR)/Core/Include
CMSIS_DSP     = $(CMSIS_DIR)/DSP
CMSIS_DSP_SRC = $(CMSIS_DSP)/Source

VENDOR_INC      = $(VENDOR_DIR)/Include
VENDOR_SRC      = $(VENDOR_DIR)/Source
VENDOR_GCC_SRC  = $(VENDOR_DIR)/Source/GCC
VENDOR_DRIVERS  = $(VENDOR_SRC)/drivers

FREE_RTOS_INC   = $(FREE_RTOS_DIR)/include
FREE_RTOS_SRC   = $(FREE_RTOS_DIR)
FREE_RTOS_PORT  = $(FREE_RTOS_DIR)/portable/ARM_CM4F
#FREE_RTOS_MEM_MNG_C   = $(FREE_RTOS_DIR)/portable/MemMang/heap_4.c
FATFS_SRC       = $(FATFS_DIR)/source

OPENOCD_SRC = $(VENDOR_DIR)/openocd/scripts/target

MCU         = cortex-m4
FLASHADDR   = 0x8000000
DEFINES     =-DARM_MATH_CM4 -D__ADUCM4050__
OPENOCD_CONF = $(OPENOCD_SRC)/aducm4050.cfg

PROJECT   = accelerometer
BUILD_DIR = src
SOURCES   = src/main.c

SOURCES  += $(VENDOR_SRC)/system_ADuCM4050.c $(VENDOR_GCC_SRC)/startup_ADuCM4050.c $(VENDOR_GCC_SRC)/reset_ADuCM4050.S
LDFLAGS   = -T$(VENDOR_GCC_SRC)/ADuCM4050.ld


SOURCES += $(wildcard $(VENDOR_DRIVERS)/*/*.c)  
#SOURCES += $(wildcard $(CMSIS_DSP_SRC)/*/*.c)    Build only neccessary part in order to fit to flash
SOURCES += $(wildcard $(FREE_RTOS_SRC)/*.c)  $(wildcard $(FREE_RTOS_PORT)/*.c) $(FREE_RTOS_MEM_MNG_C)
#SOURCES += $(wildcard $(FATFS_SRC)/*.c)  
INCLUDES = -I$(CMSIS_INC)/ -I$(CMSIS_DSP)/Include -I$(CMSIS_DSP)/Include/dsp -I$(CMSIS_DSP)/PrivateInclude \
	   -I$(VENDOR_INC)/  -I$(VENDOR_INC)/sys/ -I$(VENDOR_INC)/config/ -I$(VENDOR_INC)/drivers/ -I$(VENDOR_INC)/rtos_map/ \
	   -I$(FREE_RTOS_INC)/ -I$(FREE_RTOS_PORT) -I$(FATFS_SRC)/ -Isrc/
OPTIMIZE = s
# s ,0, 1, 2

CC       = arm-none-eabi-gcc
OBJCOPY  = arm-none-eabi-objcopy
SIZE     = arm-none-eabi-size
DEBUGGER = arm-none-eabi-gdb
OBJDUMP  = arm-none-eabi-objdump
OPENOCD  = openocd

CFLAGS = -O$(OPTIMIZE) -mcpu=$(MCU) -mthumb -fmessage-length=0 -fsigned-char  \
         -ffunction-sections -fdata-sections -ffreestanding                   \
         -fno-move-loop-invariants -Wall -Wextra -g3 -std=gnu11               \
         -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
         $(INCLUDES) $(DEFINES)
# --specs=nosys.specs -nostartfiles

OBJECTS = $(addsuffix .o, $(basename $(SOURCES)))
ELF = $(BUILD_DIR)/$(PROJECT).elf
BIN = $(BUILD_DIR)/$(PROJECT).bin
HEX = $(BUILD_DIR)/$(PROJECT).hex

all: $(BIN)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.s
	$(CC) -c $(CFLAGS) $< -o $@
	
%.o: %.S
	$(CC) -c $(CFLAGS) $< -o $@

check: $(BIN)
	$(OBJDUMP) -d $(ELF) | less

size: $(ELF)
	$(SIZE) -B $(ELF)

clean:
	rm -rf $(BIN) $(ELF) $(OBJECTS)

flash: $(BIN)
	$(OPENOCD) -f $(OPENOCD_CONF) -c "init; reset halt; flash write_image erase $(BIN) $(FLASHADDR); reset run; shutdown"

debug: $(BIN)
	$(OPENOCD) -f $(OPENOCD_CONF) &
	$(DEBUGGER) -ex "target extended-remote :3333" -ex "break main" $(ELF)
	killall openocd

.PHONY: all flash debug check size
