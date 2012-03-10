// Copyright 2009 Devrin Talen
// This file is part of ADBUSB.
// 
// ADBUSB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// ADBUSB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with ADBUSB.  If not, see <http://www.gnu.org/licenses/>.


/*! \mainpage ADBUSB Manual

This is an open-source adapter that allows you to use an old Apple keyboard! It
is a USB to ADB converter that represents the keyboard as a USB HID keyboard,
meaning no special drivers are required. It powers itself and the keyboard
through the USB connection.

Features:

- Self-powered
- Only supports one keyboard connection (no daisy-chaining, no mice, etc.)

This project makes use of the <A HREF="http://www.obdev.at/vusb/">V-USB</A> open source USB driver.

This manual is divided into the following sections:
- \subpage install
- \subpage adb

\section References

-# http://en.wikipedia.org/wiki/Apple_Desktop_Bus
-# http://mcosre.sourceforge.net/docs/adb_intro.html
-# http://geekhack.org/showwiki.php?title=Island:14290
*/


/*! \page install Installation

This project includes software for an Atmel AVR microcontroller and schematics
and designs for a PCB.

\section depens Dependencies

You'll need to make sure you have gcc-avr, 
<A HREF="http://www.nongnu.org/avr-libc/">avr-libc</A>, and 
<A HREF="http://www.nongnu.org/avrdude/">avrdude</A> in
order to compile the source. On Ubuntu, you can do this with:

\verbatim
% sudo apt-get install gcc-avr avr-libc avrdude
\endverbatim

Other distributions may have a different process.

\section buildcode Building for the AVR

All of the source code is in the \c code/ subdirectory. The Makefile defines
these targets:

- \c all: Compiles all of the source files into \c main.hex.
- \c install: Uses avrdude to program the microcontroller.
- \c fixfuse: Resets fuse settings on the Mega32 to something that I know works.

You may need to update the Makefile according to your environment and the
device you are programming. The variables to look out for are \c AVR, which
defines the device (this is passed to \c avr-gcc) and \c PROGFLAGS, which are
the flags passed to \c avrdude during programming.

\section buildschem Assembling a circuit

Once you are able to program your device, you need to assemble a circuit
according to the schematics under \c schematic/, which is coming soon. I
haven't gotten far enough on the project yet to need this.

\section builddoc Documentation

The code is documented in a Doxygen-friendly format. To generate the
documentation run the following in the \c doc/ directory:

\verbatim
% make all
\endverbatim

Then open the \c doc/html/index.html file in your web browser.
*/


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/wdt.h>

#include "adb.h"
#include "keyboard.h"
#include "uart.h"
#include "usb.h"

/// File handle to UART device
static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/** \brief Reset entry point.
 *
 * At reset the device starts executing at this point. This will call
 * initializers to set up the ADB and USB interfaces. It then enters the main
 * loop and polls the ADB device and sends data on the USB interface as
 * needed.
*/
int main(void)
{
  // Initialize watchdog timer.
  wdt_disable();

  // Initialize USB.
  usb_init();

  // Initialize ADB.
  uint8_t adb_status = 0;
  uint8_t adb_len;
  uint8_t adb_data[8];
  adb_init();

  // Initialize UART.
  uart_init();
  stdout = &uart_str;
    
  printf("ADBUSB v0.4\n");
  printf("Copyright 2011-12 Devrin Talen\n");

  while(1) {
    usbPoll();
    /* USB phase. */
    if (usbInterruptIsReady()) {
      keybReportBuffer.meta = kb_usbhid_modifiers();
      kb_usbhid_keys(keybReportBuffer.b);
      usbSetInterrupt((void *)&keybReportBuffer, sizeof(keybReportBuffer));
      keybReportBuffer.b[0] = 0;
    }
    /* ADB phase. */
    if (usbInterruptIsReady()) {
      //adb_status = adb_command(2, ADB_CMD_TALK, 0);
    }
    /* if (adb_status != 0) { */
    /*   adb_status = adb_read_data(&adb_len, adb_data); */
    /*   if (adb_status == 0) { */
    /* 	if (adb_len == 16) { */
    /* 	  kb_register(adb_data[0]); */
    /* 	} else { */
    /* 	  //kb_reset(); */
    /* 	} */
    /*   } */
    /* } */
  }

  return 0;
}
