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

#include <avr/interrupt.h>
#include "usbdrv/usbdrv.h"
#include "usbdrv/oddebug.h"

/// Keyboard HID Report Descriptor
/**
    This is copied shamelessly from the HID-Keys example.
*/
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

/// Keyboard HID Report
/**
    The structure of this report is determined by the report descriptor. In
    this case, it's a 2b value where the top byte is the modifier key, and the
    bottom byte is the keycode.
*/
uint8_t hid_report[2];

/// Keyboard idle rate
/**
    For some reason the HID spec wants us to track this.
*/
uint8_t idle_rate;

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
    usbMsgPtr = hid_report;

    // Handle class request type
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        // wValue: ReportType (highbyte), ReportID (lowbyte)
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            // we only have one report type, so don't look at wValue
            return sizeof(hid_report);
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
    usbInit();
    sei();

    hid_report[0] = 0;
    hid_report[1] = 0;

    return;
}
