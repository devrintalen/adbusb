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

#include "usbdrv/usbdrv.h"
#include "usbdrv/oddebug.h"

PROGMEM char usbHidReportDescriptor[];

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
    //usbMsgPtr = reportBuffer;

    /* class request type */
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        /* wValue: ReportType (highbyte), ReportID (lowbyte) */
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            /* we only have one report type, so don't look at wValue */
            //buildReport(keyPressed());
            //return sizeof(reportBuffer);
            return USB_NO_MSG;
        } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
            //usbMsgPtr = &idleRate;
            //return 1;
            return USB_NO_MSG;
        } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
            //idleRate = rq->wValue.bytes[1];
        }
    } else {
        /* no vendor specific requests implemented */
    }
    return 0;
}

