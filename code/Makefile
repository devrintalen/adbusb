AVR = atmega32
OBJECTS=main.o adb.o uart.o

CC=avr-gcc
CFLAGS=-Wall -g -O2 -mmcu=$(AVR) -D F_CPU=16000000UL
OBJCOPY=avr-objcopy
OBJCOPYFLAGS=-j .text -j .data -O ihex
PROGRAMMER=avrdude
PROGFLAGS=-q -p m32 -P /dev/ttyUSB0 -c stk500v2

%.o : %.c
	$(CC) $(CFLAGS) -c $^

%.hex : %.o
	$(OBJCOPY) $(OBJCOPYFLAGS) a.out $@

all: $(OBJECTS)
	$(CC) $(CFLAGS) $^

install: main.hex
	$(PROGRAMMER) $(PROGFLAGS) -e -U flash:w:main.hex

fixfuse:
	$(PROGRAMMER) $(PROGFLAGS) -e -U lfuse:w:0b11100000:m
	
clean:
	rm -f *.o *.elf *.hex a.out
