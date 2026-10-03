# ==========================================
# Hardware & port configuration
# ==========================================
MCU          = atmega328p
F_CPU        = 16000000UL
PORT         = /dev/ttyUSB0
BAUD_UPLOAD  = 115200

# ==========================================
# Toolchain
# ==========================================
CC           = avr-gcc
OBJCOPY      = avr-objcopy
AVRDUDE      = avrdude

# ==========================================
# Project files
# ==========================================
TARGET       = adc
SRC          = adc.c

# ==========================================
# Compiler flags
# ==========================================
CFLAGS       = -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Wall

# ==========================================
# Build targets
# ==========================================
all: compile

compile: $(SRC)
	@echo "=== Compiling C source to object file ==="
	$(CC) $(CFLAGS) -c $(SRC) -o $(TARGET).o

	@echo "=== Linking object file to ELF ==="
	$(CC) -g -mmcu=$(MCU) -o $(TARGET).elf $(TARGET).o

	@echo "=== Creating HEX file ==="
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
	@echo "Success: $(TARGET).hex is ready."

upload: compile
	@echo "=== Flashing firmware to Arduino ==="
	sudo $(AVRDUDE) -F -V -c arduino -p m328p -P $(PORT) -b $(BAUD_UPLOAD) -U flash:w:$(TARGET).hex

clean:
	rm -f $(TARGET).o $(TARGET).elf $(TARGET).hex
	@echo "=== Workspace cleaned ==="
