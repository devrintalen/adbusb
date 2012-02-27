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
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "adb.h"

/// Address of last polled device
uint8_t last_device;

/// State values
enum adb_states {
  ADB_STATE_IDLE = 0,
  ADB_STATE_TX_ATTN,
  ADB_STATE_TX_SYNC,
  ADB_STATE_TX_BIT_LOW,
  ADB_STATE_TX_BIT_HIGH,
  ADB_STATE_RX_WAIT,
  ADB_STATE_RX_LOW,
  ADB_STATE_RX_HIGH
};
/// Current state
uint8_t adb_state;

// State information for transmitting a byte
/// Byte to transmit
uint8_t adb_tx_data;
/// Index of bit to transmit
int8_t adb_tx_index;

// State information for receiving data
/// Number of bits received
uint8_t adb_rx_count;
/// Last received bit
uint8_t adb_rx_bit;
/// Received data
uint8_t adb_rx_data[8];
/// Length of last received low pulse
uint8_t adb_rx_low_duration;

ISR(TIMER0_COMP_vect)
{
  PORTA &= ~(_BV(1));

  switch (adb_state) {

  case ADB_STATE_TX_ATTN:
    adb_state = ADB_STATE_TX_SYNC;
    ADB_PORT = ADB_TX_1;
    // Set up timer for 70us.
    TCCR0 = (TCCR0 & 0xF8) | 0x2; // prescalar = 8
    TCNT0 = 0;
    OCR0 = 70 / 0.5;
    break;

  case ADB_STATE_TX_SYNC:
  case ADB_STATE_TX_BIT_HIGH:
    if (adb_tx_index == -2) {
      adb_state = ADB_STATE_RX_WAIT;
      // Set up port to receive data
      ADB_PORT = ADB_TX_1;
      DDRB = 0x00;
      // Start counting time and disable the interrupt
      TCCR0 = (TCCR0 & 0xF8) | 0x3; // prescalar = 64
      TCNT0 = 0;
      OCR0 = 240 / 4;
      break;
    }
    ADB_PORT = ADB_TX_0;
    adb_state = ADB_STATE_TX_BIT_LOW;
    // Set up timer for either 35us or 65us.
    TCNT0 = 0;
    if (adb_tx_index == -1) {
      OCR0 = 65 / 0.5;
    } else if (((adb_tx_data >> adb_tx_index) & 0x1) == 0) {
      OCR0 = 65 / 0.5;
    } else {
      OCR0 = 35 / 0.5;
    }
    break;

  case ADB_STATE_TX_BIT_LOW:
    ADB_PORT = ADB_TX_1;
    adb_state = ADB_STATE_TX_BIT_HIGH;
    // Set up timer for either 65us or 35us.
    TCNT0 = 0;
    if (adb_tx_index == -1) {
      OCR0 = 35 / 0.5;
    } else if (((adb_tx_data >> adb_tx_index) & 0x1) == 0) {
      OCR0 = 35 / 0.5;
    } else {
      OCR0 = 65 / 0.5;
    }
    adb_tx_index--;
    break;

  case ADB_STATE_RX_WAIT:
    // 240us have elapsed since the stop bit. If an external interrupt
    // had fired by this point the state would have been modified and
    // we wouldn't get here. Re-initialize everything.
    // ... fall through to the next state since they're the same...

  case ADB_STATE_RX_HIGH:
    // About 128us have elapsed since the last bit received had started.
    // The ADB device has stopped sending data and we need to stop
    // receiving data.
    TIMSK &= ~(_BV(1)); // disable timer interrupt
    adb_state = ADB_STATE_IDLE;
    PORTA |= _BV(2);
    break;

  default:
    break;
  }

  PORTA |= _BV(1);

  return;
}

/**
   Sends a command packet with the supplied arguments. Takes care of
   asserting the attention and sync signals and the stop bit. The algorithm
   is:

   -# Assert attention signal (800us).
   -# Assert sync signal (70us).
   -# Send command byte (8 * 100us).
   -# Send stop bit (100us).
   -# Release line.

   This is handled by using hardware timer 0 to make this call non-blocking.
   Once the attention signal has been started this call will return.

   @param[in]  address Device address.
   @param[in]  command Command to send.
   @param[in]  reg     Register to read/write.
   @return     0 for success.
*/
int8_t adb_command(uint8_t address, uint8_t command, uint8_t reg)
{
  PORTA &= ~(_BV(0));
  DDRB = 0xff;

  // Construct command byte
  adb_tx_data = 0;
  adb_tx_data |= address << 4;
  adb_tx_data |= command << 2;
  adb_tx_data |= reg;
  adb_tx_index = 7; // data is sent MSB first

  // Start the state machine
  adb_state = ADB_STATE_TX_ATTN;
  ADB_PORT = ADB_TX_0;
  // Kick off the timer for 800us
  TCCR0 = (TCCR0 & 0xF8) | 0x3; // prescalar = 64
  TCNT0 = 0;
  OCR0 = 800 / 4;
  TIMSK |= _BV(1);

  PORTA |= _BV(0);
  return 0;
}

/**
   External interrupt on ADB pin. Triggered when an ADB device starts 
   transmitting data to the processor.
*/
ISR(INT2_vect) {
  PORTA &= ~(_BV(1));

  switch (adb_state) {

  case ADB_STATE_RX_WAIT:
    TIMSK &= ~(_BV(1));
    TCCR0 = 0xa;
    OCR0 = 255;
    adb_rx_count = 0;
    PORTA &= ~(_BV(2));
    // Purposefully fall through to the next state...

  case ADB_STATE_RX_LOW:
    // Begin counting from 0 to determine how long the low pulse is.
    TCNT0 = 0;
    adb_state = ADB_STATE_RX_HIGH;
    break;

  case ADB_STATE_RX_HIGH:
    // Capture the duration of the low pulse.
    adb_rx_low_duration = TCNT0;
    // Record the bit.
    adb_rx_count++;
    if (adb_rx_low_duration < (40 * 2)) {
      adb_rx_bit = 0;
    } else {
      adb_rx_bit = 1;
    }
    adb_rx_data[adb_rx_count / 8] |= adb_rx_bit << (adb_rx_count % 8);
    adb_state = ADB_STATE_RX_LOW;
    break;

  default:
    break;
  }

  PORTA |= _BV(1);

  return;
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
  PORTA = 0xFF;

  // Reach steady state then reset devices
  ADB_PORT = ADB_TX_1;
  _delay_ms(1000.0);
  ADB_PORT = ADB_TX_0;
  _delay_ms(4.0);
  ADB_PORT = ADB_TX_1;

  // Initialize to default keyboard address
  // keyboard: 0x2
  // mouse: 0x3
  last_device = 2;

  // Initialize the rx resources
  adb_rx_count = 0;
  memset(adb_rx_data, 0, 8*sizeof(uint8_t));

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
  uint8_t poll_result = 0;

  // Initialize length
  *len = 0;

  // If ADB is non-idle then return 1
  if (adb_state != ADB_STATE_IDLE) {
    return 1;
  }

  // Begin a poll command
  adb_command(last_device, ADB_CMD_TALK, 0);

  return poll_result;
}
