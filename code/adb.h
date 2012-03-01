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

/** \file adb.h
    \brief Global routines for the ADB interface.
*/

#ifndef __inc_adb__
#define __inc_adb__

/// Output port
#define ADB_PORT PORTB
/// Output low value
#define ADB_TX_0 0x0
/// Output high value
#define ADB_TX_1 0x4

/// 2b code for a flush command.
#define ADB_CMD_FLUSH 0
/// 2b code for a listen command.
#define ADB_CMD_LISTEN 2
/// 2b code for a talk command.
#define ADB_CMD_TALK 3

int8_t adb_command(uint8_t address, uint8_t command, uint8_t reg);
int8_t adb_read_data(uint8_t *len, uint8_t *buff);
int8_t adb_init(void);

#endif
