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

/** \file adb.c
    \brief ADB interface source.

    Defines routines and interrupts specific to the ADB interface.
*/

#include <stdint.h>
#include <avr/io.h>
/// Clock speed must be defined for delay.h
/**
    With a clock at 16MHz the maximum time that can be delayed is:
    - for μs: \f$768/16 = 48\f$
    - for ms: \f$262.14/16 = 16.38\f$
*/
#define F_CPU 16000000UL
#include <util/delay.h>

#include "adb.h"

/// 2b code for a flush command.
#define ADB_CMD_FLUSH 0
/// 2b code for a listen command.
#define ADB_CMD_LISTEN 2
/// 2b code for a talk command.
#define ADB_CMD_TALK 3

/// Time (in μs) for an attention signal.
#define ADB_TIME_ATTN 800.0
/// Time (in μs) for a sync signal.
#define ADB_TIME_SYNC 70.0
/// Time (in μs) for an individual bit.
#define ADB_TIME_BIT 100.0

/// Time (in μs) to hold the significant part of a bit.
#define ADB_TIME_BIT_LONG 65.0
/// Time (in μs) to hold the non-significant part of a bit.
#define ADB_TIME_BIT_SHORT 35.0

/// Macro to delay 35 us
#define ADB_DELAY_35 _delay_us(35.0);
/// Macro to delay 65 us
#define ADB_DELAY_65 _delay_us(35.0); _delay_us(30.0);
/// Macro to delay 70 us
#define ADB_DELAY_70 _delay_us(35.0); _delay_us(35.0);
/// Macro to delay 800 us
#define ADB_DELAY_800 _delay_ms(0.80);

/// Address of last polled device
uint8_t last_device;

/// Send a bit
int8_t adb_txbit(uint8_t bit)
{
    // Lower line
    PORTC = 0;

    // Delay for: 0 -> 65us, 1 -> 35us
    if (bit == 0) {
        ADB_DELAY_65;
    } else {
        ADB_DELAY_35;
    }

    // Raise line
    PORTC = 1;

    // Delay for: 0 -> 35us, 1 -> 65us
    if (bit == 0) {
        ADB_DELAY_35;
    } else {
        ADB_DELAY_65;
    }

    return 0;
}

/// Send a command byte
int8_t adb_txbyte(uint8_t command)
{
    /// Loop iterator
    uint8_t i;

    // For each of the 8 bits
    for(i = 0; i < 8; i++)
    {
        // Send leftmost bit
        if (command & 0x80) {
            adb_txbit(1);
        } else {
            adb_txbit(0);
        }

        command = command << 1;
    }

    return 0;
}

/// Initializes resources used by the ADB host interface.
int8_t adb_init(void)
{
    // Configure port c for output
    DDRC = 0xFF;

    // Raise line for idle state
    PORTC = 1;

    last_device = 3;

    return 0;
}

/// Polls the active device for new data.
int8_t adb_poll(void)
{
    /*  The algorithm for polling devices is:
        1. Assert attention signal.
        2. Assert sync signal.
        3. Send command byte.
        4. Send stop bit.
        5. Release line.
    */

    /// Needed for delay macro
    double delay_amt;

    /// Command packet
    uint8_t command = 0;

    // Construct the command packet
    command |= last_device << 4; // address
    command |= ADB_CMD_TALK << 2; // command
    command |= 0; // register
    
    // Send attention signal
    PORTC = 0;
    ADB_DELAY_800;

    // Send sync signal
    PORTC = 1;
    ADB_DELAY_70;

    // Send command byte
    adb_txbyte(command);

    // Send stop bit
    adb_txbit(0);

    // Release line
    PORTC = 1;

    return 0;
}
