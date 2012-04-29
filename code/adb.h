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

\verbatim
   _      __        _    ____
0:  |____|  |_   1:  |__|    |_
      65  35          35  65
\endverbatim

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

After initialization the flow is:

-# Attention signal (low for 800us).
-# Sync signal (high for 70us).
-# Command packet
   - 8 bits (100us each)
   - Stop bit (same as a 0)
-# Tlt signal (stop-to-start time, high for 160 to 240us).
-# Data packet
   - 2-8 bytes

Devices ask for attention with a service request (Srq) signal. This is
a low signal for 300us. An Srq can only be sent by a device during the 
stop bit cell time if the request is not for it (to a different address).

The default active device is \c 0x3. The host should continuously poll
the last active device (that asserted Srq). The device will only 
respond if it has data to send.
*/

/*!
  \file adb.h
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

/**
   Send a command packet and receive data if sent. Constructs a command
   packet and sent according to the ADB specification:
  
   -# Assert attention signal (800us).
   -# Assert sync signal (70us).
   -# Send command byte (8 * 100us).
   -# Send stop bit (100us).
   -# Release line.
  
   After sending the command packet, this will wait for data to be 
   returned. If the device begins sending data then this will begin 
   recording it. The specification states that we must wait between
   160us and 240us for the device to start. A response packet looks like:
  
   -# Start bit (1)
   -# Two to eight bytes of data, sent in order of 0 to 7. Each byte is
      sent MSB first.
   -# Stop bit (0)
  
   Returned data is stored in adb_rx_data. The number of bits received
   is available in adb_rx_count.
  
   This code uses timer0 and INT2 to make this call non-blocking.
   Once the attention signal has been started this call will return.
   Successive calls will return non-zero status until the state machine
   reaches idle again.
  
   @param[in]  address Device address.
   @param[in]  command Command to send.
   @param[in]  reg     Register to read/write.
   @return     0 for success.
*/
int8_t adb_command(uint8_t address, uint8_t command, uint8_t reg);


/**
   Read received data. After an ADB command completes any received
   data will be stored into a temporary buffer. In order to get at
   this data this function must be called. This copies the ADB data
   and bit count from the temporary buffer into whatever is passed.

   The ADB state machine will not send another command while there 
   is data waiting in the buffer. Likewise, this function will return
   a non-zero value if the ADB state machine is not done sending a
   command.
  
   @param[in] *len Pointer to store the bit count.
   @param[in] buff Pointer to a uint8_t[8] buffer.
   @return         0 if data was copied, 1 otherwise.
*/
int8_t adb_read_data(uint8_t *len, uint8_t *buff);


/**
   Initialize resources. This routine initializes the microprocesser 
   resources used by the ADB code and performs the ADB bringup sequence. 
   This consists of:
  
   -# Raise the line and remain stable for 1s.
   -# Perform reset pulse for 4ms (spec states 3ms, but actual Mac II
      hardware will do 4ms).
   -# Raise line.
  
   In addition to setting the ADB processor state interrupts will be enabled.
  
   @return 0 for success.
*/
int8_t adb_init(void);

#endif
