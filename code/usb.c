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

/** \file usb.c
    \brief USB high-level driver.
*/

#include "usb.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>

#include "usbdrv.h"
#include "oddebug.h"

/// Keyboard HID Report Descriptor
/**
   This is copied shamelessly from the spritesmodes code.
*/
extern const char usbHidReportDescriptor[] PROGMEM = {
  /* partial keyboard */
  0x05, 0x01,	/* Usage Page (Generic Desktop), */
  0x09, 0x06,	/* Usage (Keyboard), */
  0xA1, 0x01,	/* Collection (Application), */
  0x85, 0x01,		/* Report Id (1) */
  0x05, 0x07,            //   USAGE_PAGE (Keyboard)
  0x19, 0xe0,            //   USAGE_MINIMUM (Keyboard LeftControl)
  0x29, 0xe7,            //   USAGE_MAXIMUM (Keyboard Right GUI)

  0x15, 0x00,            //   LOGICAL_MINIMUM (0)
  0x25, 0x01,            //   LOGICAL_MAXIMUM (1)
  0x75, 0x01,            //   REPORT_SIZE (1)
  0x95, 0x08,            //   REPORT_COUNT (8)
  0x81, 0x02,            //   INPUT (Data,Var,Abs)

  0x05, 0x07,		/* Usage Page (Key Codes), */
  0x95, 0x04,		/* Report Count (4), */
  0x75, 0x08,		/* Report Size (8), */
  0x15, 0x00,		/* Logical Minimum (0), */
  0x25, 0x75,		/* Logical Maximum(117), */
  0x19, 0x00,		/* Usage Minimum (0), */
  0x29, 0x75,		/* Usage Maximum (117), */
  0x81, 0x00,		/* Input (Data, Array),               ;Key arrays (4 bytes) */
  0xC0,		/* End Collection */

  /* mouse */
  0x05, 0x01,	/* Usage Page (Generic Desktop), */
  0x09, 0x02,	/* Usage (Mouse), */
  0xA1, 0x01,	/* Collection (Application), */
  0x09, 0x01,	/*   Usage (Pointer), */
  0xA1, 0x00,	/*   Collection (Physical), */
  0x05, 0x09,		/* Usage Page (Buttons), */
  0x19, 0x01,		/* Usage Minimum (01), */
  0x29, 0x03,		/* Usage Maximun (03), */
  0x15, 0x00,		/* Logical Minimum (0), */
  0x25, 0x01,		/* Logical Maximum (1), */
  0x85, 0x02,		/* Report Id (2) */
  0x95, 0x03,		/* Report Count (3), */
  0x75, 0x01,		/* Report Size (1), */
  0x81, 0x02,		/* Input (Data, Variable, Absolute), ;3 button bits */
  0x95, 0x01,		/* Report Count (1), */
  0x75, 0x05,		/* Report Size (5), */
  0x81, 0x01,		/* Input (Constant),                 ;5 bit padding */
  0x05, 0x01,		/* Usage Page (Generic Desktop), */
  0x09, 0x30,		/* Usage (X), */
  0x09, 0x31,		/* Usage (Y), */
  0x15, 0x81,		/* Logical Minimum (-127), */
  0x25, 0x7F,		/* Logical Maximum (127), */
  0x75, 0x08,		/* Report Size (8), */
  0x95, 0x02,		/* Report Count (2), */
  0x81, 0x06,		/* Input (Data, Variable, Relative), ;2 position bytes (X & Y) */
  0xC0,		/*   End Collection, */
  0xC0,		/* End Collection */
};

/// Keyboard idle rate
/**
   For some reason the HID spec wants us to track this.
*/
static uint8_t idle_rate;

/// Initialize USB hardware
/**
   Initialize any resources needed by the USB code and hardware.
*/
void usb_init()
{
  uint8_t i;

  //odDebugInit();

  usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
  i = 0;
  while(--i){             /* fake USB disconnect for > 250 ms */
    _delay_ms(1.0);
  }
  usbDeviceConnect();

  usbInit();
  sei();

  return;
}

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
      usbMsgPtr = (void *)&mouseReportBuffer;
      return sizeof(mouseReportBuffer);
    } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
      usbMsgPtr = &idle_rate;
      return sizeof(idle_rate);
    } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
      idle_rate = rq->wValue.bytes[1];
    }
  }
  return 0;
}
