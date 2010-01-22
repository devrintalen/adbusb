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
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
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
/// Macro to delay 200 us
#define ADB_DELAY_200 _delay_us(200.0);
/// Macro to delay 800 us
#define ADB_DELAY_800 _delay_us(800.0);

/// Output low value
#define ADB_TX_LOW 0x0
/// Output high value
#define ADB_TX_HIGH 0x1

/// Address of last polled device
uint8_t last_device;
/// Received data buffer (2-8 bytes)
uint8_t adb_rx_buff[8];
/// Received data length (in bits)
uint8_t adb_rx_len;

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
    PORTC = ADB_TX_LOW;

    // Delay for: 0 -> 65us, 1 -> 35us
    if (bit == 0) {
        ADB_DELAY_65;
    } else {
        ADB_DELAY_35;
    }

    // Raise line
    PORTC = ADB_TX_HIGH;

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
    Places the MCU into a receive state and waits for a response. If none is
    given then it will return 1, if it receives data it will fill the global
    buffer with data and return 0.

    This function will wait for 240us to receive data before it returns. After
    160us it enables the external interrupt to catch a response from the
    device. If and when this interrupt gets triggered it will begin a series of
    timers to record the data. The external interrupt will be reconfigured to
    watch for the rising edge. When this gets hit it will read the
    timer/counter to determine if it received a 0 or 1 (65us for a 0, 35us for
    a 1). The timer will interrupt at 100us intervals. If the timer fires and
    no bit was recorded during the 100us it will signal the end of the receive.

    @return     0 if data is received, 1 if not.
*/
int8_t adb_rx()
{
    // Initialize resources
    adb_rx_len = 0;
    memset((void*)adb_rx_buff, 0, 8);

    // Wait for 160us before receiving data.
    ADB_DELAY_160;

    // Enable external interrupt on data line (int0)
    GICR |= _BV(INT0);

    // Wait for 80us for device to respond. If it does it will enter the int0
    // handler, receive the data, and return here. If the receiving data flag
    // is high then this will continue to wait.
    ADB_DELAY_80;
    while(adb_rx) ;

    // Disable external interrupt on data line (int0)
    GICR &= ~_BV(INT0);

    // Return 0 if we received data
    if (adb_rx_len > 0)
        return 0;
    else
        return 1;
}

/// External interrupt 0 vector
/**
    This gets triggered under two conditions:

    -# The MCU begins receiving data from the device. If this is the case it
    should reconfigure to trigger on the rising edge for the next condition:
    -# Device has transmitted a bit.
*/
ISR(INT0_vect)
{
    if (!adb_rx)
    {
        adb_rx = true;
        MCUCR = 3; // Generate interrupt on rising edge
        TCCR0 |= _BV(WGM01) | _BV(CS01); // CTC, clk/8
        OCR0 = 99; // Generate interrupt every 100us
        return;
    }

    // At this point we know we are in the middle of receiving data. Compare
    // the value of timer0 to 50 to see if if we just received a 0 or 1. Set
    // the bit flag to indicate we received a bit.
    adb_rx_data = true;
    if (TCNT0 < foo)
        adb_rx_bit = 0;
    else
        adb_rx_bit = 1;

    return;
}

/// Timer0 compare interrupt
/**
    This is triggered during a receive sequence every 100us. It will check to
    make sure a bit was received (rx_data flag) and what the value was
    (rx_bit) and add it to the buffer. If a bit was not received (rx_data is
    clear) then the device has stopped sending data and the receive sequence
    should be ended.
*/
ISR(TIMER0_COMP_vect)
{
    if (!adb_rx_data)
    {
        adb_rx = false;
        TCCR0 = 0; // Disable timer0
        return;
    }

    // Increment the bit counter and store the bit received into the buffer.
    adb_rx_len++;
    adb_rx_buff[adb_rx_len / 8] = (adb_rx_buff[adb_rx_len / 8] << 1) | rx_bit;

    adb_rx_data = false;
    return;
}

/// Send a command packet.
/**
    Constructs a command packet out of the supplied arguments. Takes care of
    asserting the attention and sync signals, and the stop bit. The algorithm
    is:

    -# Assert attention signal.
    -# Assert sync signal.
    -# Send command byte.
    -# Send stop bit.
    -# Release line.

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

    DDRC = 0xFF;

    // Send attention signal
    PORTC = ADB_TX_LOW;
    ADB_DELAY_800;

    // Send sync signal
    PORTC = ADB_TX_HIGH;
    ADB_DELAY_70;

    // Send command byte
    adb_txbyte(packet);

    // Send stop bit
    adb_txbit(0);

    // Release line
    PORTC = ADB_TX_HIGH;

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
    DDRC = 0xFF;

    // Reach steady state then reset devices
    // TODO this will probably have to change when USB is added.
    PORTC = ADB_TX_HIGH;
    _delay_ms(1000.0);
    PORTC = ADB_TX_LOW;
    _delay_ms(4.0);
    PORTC = ADB_TX_HIGH;

    // Initialize to default mouse address
    last_device = 3;

    // Enable interrupts
    sei();

    // External interrupt on falling edge of int0
    MCUCR = 2;
    GICR &= ~(1 << 6); // disable interrupt for now

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
    // Initialize length
    *len = 0;

    // Send a poll command
    adb_command(last_device, ADB_CMD_TALK, 3);

    // Receive data. If any is received copy the data to the buffer passed in
    // and return the correct length.
    if (adb_rx() == 0)
    {
        *len = adb_rx_len;
        memcpy((void*)buff, (void*)adb_rx_buff, 8);
        return 1;
    }

    return 0;
}

/// Enumerate all attached devices
/**
    Perform an initialization routine that searches for any attached devices
    and reassigns them to a known address.

    - Mice will be found at 0x3
    - Keyboards will be found at 0x2

    This implementation will start assigning devices to sequential addresses
    beginning at 0x8 - meaning that only 8 devices are supported.
*/
int8_t adb_enumerate(void)
{
    uint8_t data[8];
    uint8_t len = 0;

    // Search for any attached keyboards
    adb_command(0x3, ADB_CMD_TALK, 3);
    adb_rx(data, &len);
    if (len)
    {
        // A keyboard has responded and has moved to a random higher address.
        // This address is returned in data[0]. We now tell that keyboard to
        // move to an address we want.
        adb_command(data[0], ADB_CMD_LISTEN, 0);
        // TODO move keyboard
    }

    return 0;
}
