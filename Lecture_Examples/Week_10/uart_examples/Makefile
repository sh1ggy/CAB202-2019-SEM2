# Topic 10 Makefile (with USB Serial and UART)
# Lawrence Buckingham, October  2017.
# Queensland University of Technology.

# Replace these targets with your target (hex file) name, including the .hex part.

TARGETS = \
	uart_hello.hex \
	uart1.hex	\
	uart2.hex

# Set the name of the folder containing libcab202_teensy.a
CAB202_TEENSY_FOLDER=../../../software/cab202_teensy

# Set the name of the folder containing usb_serial.o
USB_SERIAL_FOLDER =../../../software/usb_serial
USB_SERIAL_OBJ =$(USB_SERIAL_FOLDER)/usb_serial.o

# Set the name of the folder containing uart.o
UART_FOLDER =../../../software/uart
UART_OBJ =$(UART_FOLDER)/uart.o


# ---------------------------------------------------------------------------
#	Leave the rest of the file alone.
# ---------------------------------------------------------------------------

all: $(TARGETS)

TEENSY_LIBS = $(USB_SERIAL_OBJ) $(UART_OBJ) -lcab202_teensy -lprintf_flt -lm 

TEENSY_DIRS =-I$(CAB202_TEENSY_FOLDER) \
	-I$(USB_SERIAL_FOLDER) -L$(CAB202_TEENSY_FOLDER) \
	-I$(UART_FOLDER) -L$(UART_FOLDER)

TEENSY_FLAGS = \
	-std=gnu99 \
	-mmcu=atmega32u4 \
	-DF_CPU=8000000UL \
	-funsigned-char \
	-funsigned-bitfields \
	-ffunction-sections \
	-fpack-struct \
	-fshort-enums \
	-Wall \
	-Werror \
	-Wl,-u,vfprintf \
	-Os 

ZDK_FLAGS =  -lm -Werror -Wall -std=gnu99

clean:
	for f in $(TARGETS); do \
		if [ -f $$f ]; then rm $$f; fi; \
		if [ -f $$f.elf ]; then rm $$f.elf; fi; \
		if [ -f $$f.obj ]; then rm $$f.obj; fi; \
	done

rebuild: clean all

%.hex : %.c
	avr-gcc $< $(TEENSY_FLAGS) $(TEENSY_DIRS) $(TEENSY_LIBS) -o $@.obj
	avr-objcopy -O ihex $@.obj $@

%.exe : %.c
	gcc $< $(ZDK_FLAGS) -o $@
