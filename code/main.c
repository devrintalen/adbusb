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

/// Proof of concept function to type "Hello world!"
void type_helloworld()
{
    static uint8_t index = 0;

    if (index == 2) {
        keybReportBuffer.meta = 0x00; // left shift = 2
        keybReportBuffer.b[0] = USB_KEY_H;
    } else if (index == 4) {
        keybReportBuffer.b[0] = USB_KEY_E;
    } else if (index == 6) {
        keybReportBuffer.b[0] = USB_KEY_L;
    } else if (index == 8) {
        keybReportBuffer.b[0] = USB_KEY_L;
    } else if (index == 10) {
        keybReportBuffer.b[0] = USB_KEY_O;
    } else if (index == 12) {
        keybReportBuffer.b[0] = USB_KEY_SPACE;
    } else if (index == 14) {
        keybReportBuffer.b[0] = USB_KEY_W;
    } else if (index == 16) {
        keybReportBuffer.b[0] = USB_KEY_O;
    } else if (index == 18) {
        keybReportBuffer.b[0] = USB_KEY_R;
    } else if (index == 20) {
        keybReportBuffer.b[0] = USB_KEY_L;
    } else if (index == 22) {
        keybReportBuffer.b[0] = USB_KEY_D;
    } else {
      //keybReportBuffer.meta = 0x00;
      //keybReportBuffer.b[0] = 0;
    }

    index++;

    return;
}

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
    //uint8_t adb_buff[8];
    //uint8_t adb_len;
    //uint8_t adb_status;
    //adb_init();

    //uart_init();
    //stdout = &uart_str;
    
    //printf("ADBUSB v0.1\n");
    //printf("Copyright 2011 Devrin Talen\n");

    while(1)
    {
        //_delay_ms(10.0);
        //adb_status = adb_poll(adb_buff, &adb_len);
        wdt_reset();
        usbPoll();
        if (usbInterruptIsReady()) {
            type_helloworld();
            DBG1(0x03, 0, 0);   /* debug output: interrupt report prepared */
            usbSetInterrupt((void *)&keybReportBuffer, sizeof(keybReportBuffer));
	    keybReportBuffer.b[0] = 0;
        }
    }

    return 0;
}
