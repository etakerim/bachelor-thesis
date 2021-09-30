CMSIS_SRC = CMSIS/src/stm32f303
CMSIS_INC = CMSIS/include/stm32f3xx

MCU = cortex-m4
FLASHADDR = 0x8000000
DEFINES =-DSTM32F303xC

PROJECT = msgboard
BUILD_DIR=src
SOURCES=src/accelerometer.c
SOURCES += libstm/gpio.c libstm/i2c.c libstm/usart.c libstm/queue.c

SOURCES += $(CMSIS_SRC)/system.c  $(CMSIS_SRC)/startup.c $(CMSIS_SRC)/vectors.c
INCLUDES = -ICMSIS/include/generic/ -I$(CMSIS_INC)/ -Ilibstm/
LDFLAGS = -T$(CMSIS_SRC)/memory.ld
OPTIMIZE = s
# s ,0, 1, 2

CC       = arm-none-eabi-gcc
OBJCOPY  = arm-none-eabi-objcopy
SIZE     = arm-none-eabi-size
DEBUGGER = arm-none-eabi-gdb
OBJDUMP  = arm-none-eabi-objdump
ST_FLASH  = st-flash
ST_UTIL  = st-util

CFLAGS = -O$(OPTIMIZE) -mcpu=$(MCU) -mthumb -fmessage-length=0 -fsigned-char  \
         -ffunction-sections -fdata-sections -ffreestanding                   \
         -fno-move-loop-invariants -Wall -Wextra -g3 -std=gnu11               \
         --specs=nosys.specs -nostartfiles -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
         $(INCLUDES) $(DEFINES)
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

# https://github.com/texane/stlink
flash: $(BIN)
	$(ST_FLASH) write $(BIN) $(FLASHADDR)

debug: $(BIN)
	$(ST_UTIL) &
	$(DEBUGGER) -ex "target extended-remote :4242" -ex "load" $(ELF)
	killall st-util

check: $(BIN)
	$(OBJDUMP) -d $(ELF) | less

size: $(ELF)
	$(SIZE) -B $(ELF)

clean:
	rm -rf $(BIN) $(ELF) $(OBJECTS)

.PHONY: all flash debug check size
