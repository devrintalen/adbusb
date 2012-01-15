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

/** \file main.c
    \brief Main program source.

    This file contains global routines and interrupts.
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "adb.h"
#include "keyboard.h"
#include "uart.h"
#include "usb.h"

/// File handle to UART device
static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

/// Reset entry point.
/**
    At reset the device starts executing at this point. This will call
    initializers to set up the ADB and USB interfaces. It then enters the main
    loop and polls the ADB device and sends data on the USB interface as
    needed.
*/
int main(void)
{
    wdt_disable();

    // Initialize USB
    usb_init();

    // Initialize ADB
    uint8_t adb_buff[8];
    uint8_t adb_len;
    uint8_t adb_status;
    adb_init();

    uart_init();
    stdout = &uart_str;
    
    printf("ADBUSB v0.4\n");
    printf("Copyright 2011-12 Devrin Talen\n");

    //uint8_t dbg_index = 0;
    while(1)
    {
      //_delay_ms(10.0);
      adb_status = adb_poll(adb_buff, &adb_len);
      if (adb_len > 0) {
      /* 	if (~adb_buff[0] & 0x80) { */
      /* 	  printf("%c", kb_dtoa(adb_buff[0])); */
      /* 	} */
	kb_register(adb_buff[0]);
      }
      wdt_reset();
      usbPoll();
      if (usbInterruptIsReady()) {
      	/* if (dbg_index == 2) { */
      	/*   keybReportBuffer.b[0] = 4; */
      	/* } else if (dbg_index == 4) { */
      	/*   keybReportBuffer.b[0] = 5; */
      	/* } else if (dbg_index == 6) { */
      	/*   keybReportBuffer.b[0] = 6; */
      	/* } else if (dbg_index == 8) { */
      	/*   keybReportBuffer.b[0] = 7; */
      	/* } */
      	/* dbg_index++; */
	keybReportBuffer.meta = kb_usbhid_modifiers();
	kb_usbhid_keys(keybReportBuffer.b);
      	usbSetInterrupt((void *)&keybReportBuffer, sizeof(keybReportBuffer));
      	keybReportBuffer.b[0] = 0;
      }
    }

    return 0;
}
