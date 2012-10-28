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

ADBUSB is an adapter that turns an ADB keyboard into a USB HID keyboard. I found an Apple Extended Keyboard II at the MIT fleamarket a few years back and wanted to be able to use it for my day-to-day programming. I looked around at the various products that turned ADB to USB, and decided to roll my own (as any computer engineer should). This project is the fruit of those efforts and I have strived to be as verbose and thorough as possible. Everything you would need to build your own, including source code, schematics, and parts lists, is included.

The latest source is always available at https://github.com/devrintalen/adbusb

Features:
- Self-powered
- Translates ADB to USB
- Works with the complete ADB keycode set (all keys, function keys, etc. are supported)
- No drivers to install (HID-compliant)
- Tested

Limitations:
- Only supports one keyboard connection (no daisy-chaining, no mice, etc.)
- Created for AVR microcontrollers only

This project makes use of the <A HREF="http://www.obdev.at/vusb/">V-USB</A> open-source USB driver.

This manual is divided into the following sections:
- \subpage install
- Circuit (coming soon)
- \subpage adb
- \subpage references

*/


/*! \page install Installation

ADBUSB runs entirely on one AVR microcontroller. I created the project on a Mega32, but you could use just about any of the Mega-series microcontrollers without modifying much. The code is relatively small and portable, topping out at around 4KB of flash.

\section depens Dependencies

You'll need to make sure you have gcc-avr, 
<A HREF="http://www.nongnu.org/avr-libc/">avr-libc</A>, and 
<A HREF="http://www.nongnu.org/avrdude/">avrdude</A> in
order to compile the source. On Ubuntu, you can do this with:

\verbatim
% sudo apt-get install gcc-avr avr-libc avrdude
\endverbatim

Other distributions may have a different process. You also need a programmer to flash the microcontroller. I'm fond of the STK500, but programmers like the AVR ISP are also popular and well-supported by avrdude.

\section buildcode Building

All of the source code is in the \c code/ subdirectory. The Makefile defines these targets:

- \c all: Compiles all of the source files into \c main.hex.
- \c install: Program the microcontroller using avrdude.
- \c fixfuse: Resets fuse settings on the Mega32 to something that I know works.
- \c clean: Remove all compiler-generated files.

You may need to update the Makefile according to your environment and the device you are programming. The variables to look out for are \c AVR, which defines the device (this is passed to \c avr-gcc) and \c PROGFLAGS, which are the flags passed to \c avrdude during programming.

In general, there are very few steps to programming the AVR. Connect your programmer, change to the \c code/ directory, and then run:

\verbatim
% make all
% make install
\endverbatim

\section builddoc Documentation

The code is documented in a Doxygen-friendly format. To generate this documentation run the following in the \c doc/ directory:

\verbatim
% make all
\endverbatim

Then open the \c doc/html/index.html file in your web browser.

*/


/*! \page references References and Further Reading

I've broken this out into different sections. This first set of links covers your basic introductory material for Apple Desktop Bus:

- http://en.wikipedia.org/wiki/Apple_Desktop_Bus
- http://www.microchip.com/stellent/idcplg?IdcService=SS_GET_PAGE&nodeId=1824&appnote=en011062
- http://mcosre.sourceforge.net/docs/adb_intro.html

This set of links is of other projects that have done similar things with Apple Desktop Bus:

- http://geekhack.org/showwiki.php?title=Island:14290
- http://spritesmods.com/?art=macsearm&page=4
- https://github.com/tmk/tmk_keyboard

And this final set is of materials that are relevant to the USB firmware and the AVR:

- http://www.obdev.at/products/vusb/hidkeys.html
- http://www.atmel.com/Images/doc2521.pdf
- http://www.usb.org/developers/docs/hidpage/

 */

/*! 
  \file main.c
  \brief Main program source.
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

/*! \brief Reset entry point.
  
  At reset the device starts executing at this point. This will call
  initializers to set up the ADB and USB interfaces. It then enters the main
  loop and polls the ADB device and sends data on the USB interface as
  needed.
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
    /* ADB phase. */
    if (usbInterruptIsReady()) {
      adb_status = adb_command(2, ADB_CMD_TALK, 0);
    }
    if (adb_status != 0) {
      adb_status = adb_read_data(&adb_len, adb_data);
      if (adb_status == 0) {
    	if (adb_len == 16) {
    	  kb_register(adb_data[0]);
    	} else {
    	  //kb_reset();
    	}
      }
    }
    /* USB phase. */
    if (usbInterruptIsReady()) {
      keybReportBuffer.meta = kb_usbhid_modifiers();
      kb_usbhid_keys(keybReportBuffer.b);
      usbSetInterrupt((void *)&keybReportBuffer, sizeof(keybReportBuffer));
      keybReportBuffer.b[0] = 0;
    }
  }

  return 0;
}
