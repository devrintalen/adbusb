AVR = atmega32

%.o : %.c
	avr-gcc -g -Os -mmcu=$(AVR) -c $^

%.elf : %.o
	avr-gcc -g -mmcu=$(AVR) -o $@ $^

%.hex : %.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

install: main.hex
	avrdude -q -p m32 -P /dev/ttyS0 -c stk500v2 -e -U flash:w:main.hex

clean:
	rm -f *.o *.elf *.hex
