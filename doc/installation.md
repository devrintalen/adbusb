Installation {#install}
============

ADBUSB runs entirely on one AVR microcontroller. I created the project
on a Mega32, but you could use just about any of the Mega-series
microcontrollers without modifying much. The code is relatively small
and portable, topping out at around 4KB of flash.

Dependencies
------------

You'll need to make sure you have [gcc-avr][1], [avr-libc][2], and
[avrdude][3] in order to compile the source. On Ubuntu, you can do
this with:

    % sudo apt-get install gcc-avr avr-libc avrdude

Other distributions may have a different process. You also need a
programmer to flash the microcontroller. I'm fond of the STK500, but
programmers like the AVR ISP are also popular and well-supported by
avrdude.

Building
--------

All of the source code is in the `code` subdirectory. The Makefile
defines these targets:

* `all`: Compiles all of the source files into `main.hex`.
* `install`: Program the microcontroller using avrdude.
* `fixfuse`: Resets fuse settings on the Mega32 to something that I know works.
* `clean`: Remove all compiler-generated files.

You may need to update the Makefile according to your environment and
the device you are programming. The variables to look out for are
`AVR`, which defines the device (this is passed to `avr-gcc`) and
`PROGFLAGS`, which are the flags passed to `avrdude` during
programming.

In general, there are very few steps to programming the AVR. Connect your programmer, change to the `code` directory, and then run:

    % make all
    % make install

Documentation
-------------

The code is documented in a Doxygen-friendly format. To generate this
documentation run the following in the `doc` directory:

    % make all

Then open the `doc/html/index.html` file in your web browser.

[1]: http://www.nongnu.org/avr-libc/
[2]: http://www.nongnu.org/avr-libc/
[3]: http://www.nongnu.org/avrdude/
