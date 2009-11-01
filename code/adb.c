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

/// Define a macro to delay for long amounts of microseconds.
/**
    With a clock at 16MHz the maximum time that can be delayed is:
    (for μs) 768/16 = 48
    This macro is needed to delay for longer amounts of time than that.
*/
#define LONG_DELAY_US(t) \
    delay_amt = t; \
    while(delay_amt > 0) { _delay_us(48.0); delay_amt -= 48.0; }
/// Define a macro to delay for long amounts of milliseconds.
/**
    With a clock at 16MHz the maximum time that can be delayed is:
    (for ms) 262.14/16 = 16.38
    This macro is needed to delay for longer amounts of time than that.
*/
#define LONG_DELAY_MS(t) \
    delay_amt = t; \
    while(delay_amt > 0) { _delay_ms(16.38); delay_amt -= 16.38; }

/// Address of last polled device
uint8_t last_device;

/// Initializes resources used by the ADB host interface.
int8_t adb_init(void)
{
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
    
    // Send attention signal
    PORTC = 0;
    LONG_DELAY_US(ADB_TIME_ATTN);

    // Send sync signal
    PORTC = 1;
    LONG_DELAY_US(ADB_TIME_SYNC);

    // Send each bit 
}
