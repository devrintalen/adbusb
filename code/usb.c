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
    \brief USB driver.
*/

#include "usbdrv/usbdrv.h"
#include "usbdrv/oddebug.h"

/// USB setup code
/**
    This does something and I have no idea what it is.
*/
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    return 0;
}

