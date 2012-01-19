=======
ADB-USB
=======

:Author: Devrin Talen
:Revision: 0.4

Overview
========
This is an open-source adapter that allows you to use an old Apple keyboard! It
is a USB to ADB converter that represents the keyboard as a USB HID keyboard,
meaning no special drivers are required. It powers itself and the keyboard
through the USB connection.

Features:

* Self-powered
* Only supports one keyboard connection (no daisy-chaining, no mice, etc.)
* Open source

This project makes use of the V-USB_ open source USB driver.

.. _V-USB: http://www.obdev.at/vusb/ 

Getting Started
---------------
This project includes software for an Atmel AVR microcontroller and schematics
and designs for a PCB. The first thing to do is compile the source for the AVR
that you want to use.

First, you'll need to make sure you have gcc-avr_, avr-libc_, and avrdude_ in
order to compile the source. On Ubuntu, you can do this with::

% sudo apt-get install gcc-avr avr-libc avrdude

All of the source code is in the ``code/`` subdirectory. The Makefile defines
these targets:

``all``
    Compiles all of the source files into ``main.hex``.

``install``
    Uses avrdude to program the microcontroller.

``fixfuse``
    Resets fuse settings on the Mega32 to something that I know works.

You may need to update the Makefile according to your environment and the
device you are programming. The variables to look out for are ``AVR``, which
defines the device (this is passed to ``avr-gcc``) and ``PROGFLAGS``, which are
the flags passed to ``avrdude`` during programming.

Once you are able to program your device, you need to assemble a circuit
according to the schematics under ``schematic/``, which is coming soon. I
haven't gotten far enough on the project yet to need this.

Documentation
-------------
The code is documented in a Doxygen-friendly format. To generate the
documentation run the following in the ``doc/`` directory::

% make all

Then open the ``doc/html/index.html`` file in your web browser.


Milestone Goals
===============
0.1:
	* Can send command packets to ADB device.
	* Breadboard circuit for communication.

0.2:
	* Successful read of data packet from keyboard.

0.3:
	* Complete ADB implementation (reads all keystrokes).

0.4:
    * Simple USB communication works (device descriptors, etc.)

0.5:
    * Keyboard HID implementation complete
        * Not connected to ADB side
        * Successfully "types" a static string

0.6:
    * ADB to USB translation code complete.

0.7:
    * PCB design complete (and parts ordered).
        * Includes ADB and USB connections.
        * Includes programming port.

0.8:
    * PCB tested and part programmed successfully.

0.9:
    * Works!
	* Code complete.

1.0:
    * Code cleanup.
    * All debug code ripped out.
    * Complete documentation.

References
==========
1.  http://en.wikipedia.org/wiki/Apple_Desktop_Bus
2.  http://mcosre.sourceforge.net/docs/adb_intro.html
3.  http://geekhack.org/showwiki.php?title=Island:14290
