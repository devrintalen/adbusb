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

/** \file keyboard.h
    \brief Global routines for the keyboard library.
*/

#ifndef __inc_keyboard__
#define __inc_keyboard__

char kb_dtoa(uint8_t d);

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
#define USB_MOD_CONTROL_LEFT    (1<<0)
#define USB_MOD_SHIFT_LEFT      (1<<1)
#define USB_MOD_ALT_LEFT        (1<<2)
#define USB_MOD_GUI_LEFT        (1<<3)
#define USB_MOD_CONTROL_RIGHT   (1<<4)
#define USB_MOD_SHIFT_RIGHT     (1<<5)
#define USB_MOD_ALT_RIGHT       (1<<6)
#define USB_MOD_GUI_RIGHT       (1<<7)

#define USB_KEY_SPACE   44

#define USB_KEY_A       4
#define USB_KEY_B       5
#define USB_KEY_C       6
#define USB_KEY_D       7
#define USB_KEY_E       8
#define USB_KEY_F       9
#define USB_KEY_G       10
#define USB_KEY_H       11
#define USB_KEY_I       12
#define USB_KEY_J       13
#define USB_KEY_K       14
#define USB_KEY_L       15
#define USB_KEY_M       16
#define USB_KEY_N       17
#define USB_KEY_O       18
#define USB_KEY_P       19
#define USB_KEY_Q       20
#define USB_KEY_R       21
#define USB_KEY_S       22
#define USB_KEY_T       23
#define USB_KEY_U       24
#define USB_KEY_V       25
#define USB_KEY_W       26
#define USB_KEY_X       27
#define USB_KEY_Y       28
#define USB_KEY_Z       29

#define USB_KEY_1       30
#define USB_KEY_2       31
#define USB_KEY_3       32
#define USB_KEY_4       33
#define USB_KEY_5       34
#define USB_KEY_6       35
#define USB_KEY_7       36
#define USB_KEY_8       37
#define USB_KEY_9       38
#define USB_KEY_0       39

#define USB_KEY_F1      58
#define USB_KEY_F2      59
#define USB_KEY_F3      60
#define USB_KEY_F4      61
#define USB_KEY_F5      62
#define USB_KEY_F6      63
#define USB_KEY_F7      64
#define USB_KEY_F8      65
#define USB_KEY_F9      66
#define USB_KEY_F10     67
#define USB_KEY_F11     68
#define USB_KEY_F12     69

#endif
