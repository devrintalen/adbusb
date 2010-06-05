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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

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
/// Macro to delay 50 us
#define ADB_DELAY_50 _delay_us(25.0); _delay_us(25.0);
/// Macro to delay 65 us
#define ADB_DELAY_65 _delay_us(35.0); _delay_us(30.0);
/// Macro to delay 70 us
#define ADB_DELAY_70 _delay_us(30.0); _delay_us(30.0);
/// Macro to delay 80 us
#define ADB_DELAY_80 _delay_us(40.0); _delay_us(40.0);
/// Macro to delay 160 us
#define ADB_DELAY_160 ADB_DELAY_80; ADB_DELAY_80;
/// Macro to delay 200 us
#define ADB_DELAY_200 _delay_us(200.0);
/// Macro to delay 800 us
#define ADB_DELAY_800 _delay_us(800.0);

/// Output port
#define ADB_PORT PORTB
/// Input pin
#define ADB_PIN PINB
/// Output low value
#define ADB_TX_LOW 0x0
/// Output high value
#define ADB_TX_HIGH 0x4

/// Address of last polled device
uint8_t last_device;
/// Received data buffer (2-8 bytes)
uint8_t adb_rx_buff[8];
/// Received data length (in bits)
uint8_t adb_rx_len;
/// Receiving data flag
uint8_t adb_rx_inprogress;
/// Current received bit
int8_t adb_rx_bit;

/// Send a bit
/**
    A bit is 100 microseconds long and starts as a low signal and gets set at
    some point. The point at which it becomes high depends on if a 0 or 1 is
    being transmitted.

    - A 0 is a 65μs low pulse followed by a 35μs high pulse
    - A 1 is a 35μs low pulse followed by a 65μs high pulse

    @param[in]  bit Value to transmit.
    @return     0 for success.
*/
int8_t adb_txbit(uint8_t bit)
{
    // Lower line
    ADB_PORT = ADB_TX_LOW;

    // Delay for: 0 -> 65us, 1 -> 35us
    if (bit == 0) {
        ADB_DELAY_65;
    } else {
        ADB_DELAY_35;
    }

    // Raise line
    ADB_PORT = ADB_TX_HIGH;

    // Delay for: 0 -> 35us, 1 -> 65us
    if (bit == 0) {
        ADB_DELAY_35;
    } else {
        ADB_DELAY_65;
    }

    return 0;
}

/// Send a command byte
/**
    Just a wrapper to transmit eight bits.

    @param[in]  command 8b value to transmit.
    @return     0 for success.
*/
int8_t adb_txbyte(uint8_t command)
{
    // Loop iterator
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

/// Receive a data packet.
/**
    Receive a packet from an ADB device. The microcontroller will wait for up
    to 240us for the device to respond, and if not will return.

    A response from an ADB device is a "1" start bit, followed by 2-8B of data,
    then a "0" stop bit. This function will store the data and bit count in
    global resources adb_rx_len and adb_rx_buff that should be consumed by the
    calling function.

    This function is blocking while data is being received. It may take a
    minimum of 240us, or a maximum of 6840us (6.8ms), depending on if and how
    much data is received.

    @return     0 if data is received, 1 if not.
*/
int8_t adb_rx()
{
    uint8_t delay = 240;
    uint8_t receiving = 0;
    uint8_t ticks = 0;
    uint8_t last_bit;

    // Initialize resources
    adb_rx_len = 0;
    memset(adb_rx_buff, 0, 8*sizeof(uint8_t));

    // Begin a busy-wait loop to delay for up to 240us. If the data line drops
    // low during this time then discard the first bit and begin receiving
    // data.
    PORTA = 0x2;
    while(delay)
    {
        _delay_us(1.0);
        delay--;
        if (bit_is_clear(ADB_PIN, 2)) {
            receiving = 1;
            while(bit_is_clear(ADB_PIN, 2)) {};
            while(bit_is_set(ADB_PIN, 2)) {};
            PORTA = 0x6;
            break;
        }
    }

    // To make things simple the rx code isn't interrupt-driven. This may be
    // changed in the future. Right now we use _delay() loops to count the
    // length of each pulse.
    while(receiving)
    {
        // This loop counts the duration of the low pulse. The time it takes
        // is used to determine if the device is sending a 0 or 1.
        ticks = 0;
        while(bit_is_clear(ADB_PIN, 2)) {
            _delay_us(1.0);
            ticks++;
        }

        // Based on the length of the low portion of the bit we know if it's a
        // 0 or 1. A 1 will be 30us, and a 0 will be 45us.
        if (ticks > 37) {
            last_bit = 0;
        } else {
            last_bit = 1;
        }
        PORTA = last_bit;

        // Store the bit into the buffer. This stores the bits in increasing
        // bit position, meaning that the data is interpreted as being sent LSB
        // first. This may be an incorrect assumption.
        // TODO Figure out if data is MSB or LSB first (probably MSB).
        // TODO This will fail when the device sends 8B because of the stop bit
        adb_rx_len++;
        uint8_t i = adb_rx_len / 8;
        assert(i <= 8);
        adb_rx_buff[i] = (adb_rx_buff[i] << 1) | last_bit;

        // Delay for the remaining time of the bit pulse. This should be 80us
        // less the length of the initial low pulse. We also monitor the data
        // line so we don't blow past the end of the bit and shave time off the
        // next one.
        int8_t remaining = 80 - ticks;
        ticks = 0;
        while(bit_is_set(ADB_PIN, 2) && (ticks < remaining)) {
            _delay_us(1.0);
            ticks++;
        }

        // If the data line still hasn't gone low, and we've waited for the
        // next bit, then the device has stopped transmitting and we're done.
        // We verify that the last bit sent is indeed the "0" stop bit and
        // shave the bit off the bit count.
        if (ticks == remaining) {
            assert(last_bit == 0);
            adb_rx_len = adb_rx_len - 1;
            receiving = 0;
        }
    }

    PORTA = 0x2;

    // Return 0 if we received data
    if (adb_rx_len)
        return 0;
    else
        return 1;
}

/// Send a command packet.
/**
    Constructs a command packet out of the supplied arguments. Takes care of
    asserting the attention and sync signals, and the stop bit. The algorithm
    is:

    -# Assert attention signal (800us).
    -# Assert sync signal (70us).
    -# Send command byte (8 * 100us).
    -# Send stop bit (100us).
    -# Release line.

    This takes approximately 1770us, or 1.7ms. This is longer than a USB frame,
    so this needs to be taken into account.

    @param[in]  address Device address.
    @param[in]  command Command to send.
    @param[in]  reg     Register to read/write.
    @return     0 for success.
*/
int8_t adb_command(uint8_t address, uint8_t command, uint8_t reg)
{
    // command byte
    uint8_t packet = 0;
    packet |= address << 4;
    packet |= command << 2;
    packet |= reg;

    // Send attention signal
    ADB_PORT = ADB_TX_LOW;
    ADB_DELAY_800;

    // Send sync signal
    ADB_PORT = ADB_TX_HIGH;
    ADB_DELAY_70;

    // Send command byte
    adb_txbyte(packet);

    // Send stop bit
    adb_txbit(0);

    // Release line
    ADB_PORT = ADB_TX_HIGH;

    return 0;
}

/// Initializes resources used by the ADB host interface.
/**
    This routine initializes the microprocesser resources used by the ADB code
    and performs the ADB bringup sequence. This consists of:

    -# Raise the line and remain stable for 1s.
    -# Perform reset pulse for 4ms (spec states 3ms, but actual Mac II
        hardware will do 4ms).
    -# Raise line.

    In addition to setting the ADB processor state interrupts will be enabled.

    @return 0 for success.
*/
int8_t adb_init(void)
{
    // Configure port for output
    DDRB = 0xFF;
    DDRA = 0xFF;

    // Reach steady state then reset devices
    // TODO this will probably have to change when USB is added.
    ADB_PORT = ADB_TX_HIGH;
    _delay_ms(1000.0);
    ADB_PORT = ADB_TX_LOW;
    _delay_ms(4.0);
    ADB_PORT = ADB_TX_HIGH;

    // Initialize to default mouse address
    last_device = 3;

    // Initialize the rx resources
    adb_rx_len = 0;
    memset(adb_rx_buff, 0, 8*sizeof(uint8_t));

    return 0;
}

/// Polls the active device for new data.
/**
    Will poll the last active device for register 0. If any data is received
    0 will be returned and the buffer filled with the data.

    @param[in]  buff    Buffer to fill with received data (8 bytes).
    @param[out] len     Length (in bits) of received data.
    @return     0 if data was received, 1 otherwise.
*/
int8_t adb_poll(uint8_t *buff, uint8_t *len)
{
    PORTA = 0x1;
    _delay_us(5.0);
    PORTA = 0x0;

    uint8_t poll_result;

    // Initialize length
    *len = 0;

    // Send a poll command
    adb_command(last_device, ADB_CMD_TALK, 0);

    // Receive data. If any is received copy the data to the buffer passed in
    // and return the correct length.
    poll_result = adb_rx();
    if (poll_result == 0)
    {
        *len = adb_rx_len;
        memcpy((void*)buff, (void*)adb_rx_buff, 8*sizeof(uint8_t));
    }

    return poll_result;
}
