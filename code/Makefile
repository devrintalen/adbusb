AVR = atmega32
OBJECTS=main.o adb.o

CC=avr-gcc
CFLAGS=-Wall -g -O2 -mmcu=$(AVR) -D F_CPU=1000000UL
OBJCOPY=avr-objcopy
OBJCOPYFLAGS=-j .text -j .data -O ihex
PROGRAMMER=avrdude
PROGFLAGS=-q -p m32 -P /dev/ttyS0 -c stk500v2 -e -U flash:w:main.hex

%.o : %.c
	$(CC) $(CFLAGS) -c $^

%.hex : %.o
	$(OBJCOPY) $(OBJCOPYFLAGS) a.out $@

all: $(OBJECTS)
	$(CC) $(CFLAGS) $^

install: main.hex
	avrdude -q -p m32 -P /dev/ttyS0 -c stk500v2 -e -U flash:w:main.hex

clean:
	rm -f *.o *.elf *.hex a.out
