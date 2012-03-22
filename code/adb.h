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

/*! \page adb Apple Desktop Bus

Apple Desktop Bus (abbreviated ADB) is a host-controlled synchronous protocol
developed for the peripherals for the original Apple computer. It is designed
to allow for daisy-chaining multiple peripherals together. To get a more
detailed overview, and for the history, read up on the
<A HREF="">Wikipedia page on ADB</A>.

We don't care about all that stuff, though. How does it work?

\section adbcables Physical connections

ADB devices are connected by a 4-pin mini-DIN connector, the same as s-video.
The layout of the pins, looking from the front, is:

\verbatim
  4 3         3 4     1 | Data
 2   1       1   2    2 | Power switch
   =           =      3 | +5V
 Female       Male    4 | Gnd
\endverbatim

\section adbprotocol Protocol

All requests are made by the host and are in bytes. Data is sent MSB first, in
order of lowest byte to highest byte. Every bit is a combination of a low pulse
and a high pulse for 100us, where the width of each determines which it is.

- \c 0 is 65us low, 35us high.
- \c 1 is 35us low, 65us high.

A request byte sent from the host is constructed in this way. The address
that is used can be any four bit value, but after reset keyboards will
default to address \c 0x2 and mice will default to \c 0x3.

\verbatim
 7        4  3  2  1  0
+----------+-----+-----+
|   Addr   | Cmd | Reg |
+----------+-----+-----+
                |      \_ 0: primary
                |         1: n/a
                |         2: n/a
                |         3: device ID
                |
                 \_______ 0: flush
                          1: n/a
                          2: listen
                          3: talk
\endverbatim

The protocol to initialize the bus is:

-# Host signals reset.
-# Host sends talk commands to addresses \c 0x2 and \c 0x3 for
   register \c 0x3 (device ID).
-# Device responds and moves to higher address (chosen randomly).
-# Host tells device to move from new address to an address it 
   chooses.


*/


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
