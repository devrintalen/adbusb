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
void kb_usbhid_keys(char *keys);
uint8_t kb_usbhid_modifiers();
uint8_t kb_register(uint8_t keycode);

#endif
