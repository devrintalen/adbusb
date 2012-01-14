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

/** \file usb.h
    \brief Global routines for the USB driver.
*/

#ifndef __inc_usb__
#define __inc_usb__

#include <stdint.h>
#include "usbdrv.h"
#include "oddebug.h"

void usb_init();

usbMsgLen_t usbFunctionSetup(uchar data[8]);

/// Keyboard HID descriptor
typedef struct {
  char id;
  char meta;
  char b[4];
} keybReport_t;

/// Mouse HID descriptor
typedef struct{
  char id;
  uchar buttonMask;
  char dx;
  char dy;
} mouseReport_t;

/// Keyboard HID report buffer
static keybReport_t keybReportBuffer = {1, 0, {0, 0, 0, 0}};

/// Mouse HID report buffer
static mouseReport_t mouseReportBuffer = {2, 0, 0, 0};

#endif
