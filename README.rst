=======
ADB-USB
=======

:Author: Devrin Talen
:Revision: 0.1

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

Source Code
-----------
All source code is kept under the ``code/`` subdirectory. To compile the
program simply use ``make all``. To program the executable to the
microcontroller use ``make install``. Clean up any compiled binaries with
``make clean``. Make sure to run ``make fixfuse`` first.

Documentation
-------------
The code is documented in a Doxygen-friendly format. To generate the
documentation run the following in the ``doc/`` directory::

    % make all

Then open the ``doc/html/index.html`` file in your web browser.

Dependencies
------------
ADBUSB is currently only supported on the ATMega32 microcontroller. Other AVRs
may work with a little tweaking, but I'm not making any promises.

To generate the binaries you will need the avr-gcc toolchain. This involves the
following:

* avr-gcc
* AVR libc

Programming the device requires the following:

* Avrdude
* Serial port _or_ USB-serial adapter
* Serial cable

Generating the documentation requires Doxygen.

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
