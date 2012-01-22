# Copyright 2009 Devrin Talen
# This file is part of ADBUSB.
# 
# ADBUSB is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# ADBUSB is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with ADBUSB.  If not, see <http://www.gnu.org/licenses/>.

AVR = atmega32
LFUSE = 0xe0
HFUSE = 0x99

OBJECTS=main.o adb.o usb.o uart.o keyboard.o usbdrv/usbdrv.o usbdrv/oddebug.o usbdrv/usbdrvasm.o 

CC=avr-gcc
CFLAGS=-Wall -g -O2
CPPFLAGS=-mmcu=$(AVR) -DF_CPU=16000000 -Iusbdrv -I. -DDEBUG_LEVEL=0
OBJCOPY=avr-objcopy
OBJCOPYFLAGS=-j .text -j .data -O ihex
PROGRAMMER=avrdude
PROGFLAGS=-p m32 -P /dev/ttyUSB0 -c stk500v2

all: main.hex

main.elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o main.elf $(OBJECTS)

main.hex: main.elf
	$(OBJCOPY) $(OBJCOPYFLAGS) main.elf main.hex
	avr-size main.hex

install: main.hex
	$(PROGRAMMER) $(PROGFLAGS) -e -U flash:w:main.hex

fixfuse:
	$(PROGRAMMER) $(PROGFLAGS) -e -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m

# Sometimes the sck value gets all screwed up and the STK500 will program
# *very* slowly. To fix this use 'make terminal' and then do 'sck 10'.
terminal:
	$(PROGRAMMER) $(PROGFLAGS) -t

clean:
	rm -f *.o usbdrv/*.o *.elf *.hex
