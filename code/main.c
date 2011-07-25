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
#include <util/delay.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include "adb.h"
#include "keyboard.h"
#include "uart.h"

#include <avr/pgmspace.h>
#include "usbdrv.h"
#include "oddebug.h"

/// File handle to UART device
static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

PROGMEM char usbHidReportDescriptor[52] = { /* USB report descriptor, size must match usbconfig.h */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xA1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM
    0x29, 0x03,                    //     USAGE_MAXIMUM
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Const,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xC0,                          //   END_COLLECTION
    0xC0,                          // END COLLECTION
};
/// Keyboard HID Report Descriptor
/**
    This is copied shamelessly from the HID-Keys example.
*/
/**
PROGMEM char usbHidReportDescriptor[35] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};
*/

/// Keyboard HID Report
/**
    The structure of this report is determined by the report descriptor. In
    this case, it's a 2b value where the top byte is the modifier key, and the
    bottom byte is the keycode.
*/
//static uint8_t hid_report[2];
typedef struct{
    uchar   buttonMask;
    char    dx;
    char    dy;
    char    dWheel;
}report_t;

static report_t reportBuffer;

static int      sinus = 7 << 6, cosinus = 0;

/* The following function advances sin/cos by a fixed angle
 * and stores the difference to the previous coordinates in the report
 * descriptor.
 * The algorithm is the simulation of a second order differential equation.
 */
static void advanceCircleByFixedAngle(void)
{
char    d;

#define DIVIDE_BY_64(val)  (val + (val > 0 ? 32 : -32)) >> 6    /* rounding divide */
    reportBuffer.dx = d = DIVIDE_BY_64(cosinus);
    sinus += d;
    reportBuffer.dy = d = DIVIDE_BY_64(sinus);
    cosinus -= d;
}

/// Keyboard idle rate
/**
    For some reason the HID spec wants us to track this.
*/
static uint8_t idle_rate;

/// Handle SETUP transactions.
/**
    Received a SETUP transaction from the USB host. This could be the start of
    a CONTROL transfer, in which case we better be ready to give the host the
    latest data from the keyboard.

    @param[in]  data    SETUP transaction data.
    @return     Length of data, or 0 if not handled.
*/
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void *)data;

    // Handle class request type
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        // wValue: ReportType (highbyte), ReportID (lowbyte)
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            // we only have one report type, so don't look at wValue
            usbMsgPtr = (void *)&reportBuffer;
            return sizeof(reportBuffer);
        } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
            usbMsgPtr = &idle_rate;
            return sizeof(idle_rate);
        } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
            idle_rate = rq->wValue.bytes[1];
        }
    }
    return 0;
}

/// Initialize USB hardware
/**
    Initialize any resources needed by the USB code and hardware.
*/
void usb_init()
{
    uint8_t i;

    odDebugInit();
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();

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

    uart_init();
    stdout = &uart_str;
    
    printf("ADBUSB v0.1\n");
    printf("Copyright 2011 Devrin Talen\n");

    while(1)
    {
        //_delay_ms(10.0);
        //adb_status = adb_poll(adb_buff, &adb_len);
        usbPoll();
        if (usbInterruptIsReady()) {
            advanceCircleByFixedAngle();
            usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
        }
    }

    return 0;
}
