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

/// State values
enum adb_states = {
  ADB_STATE_IDLE,
  ADB_STATE_TX_ATTN,
  ADB_STATE_TX_SYNC,
  ADB_STATE_TX_BIT_LOW,
  ADB_STATE_TX_BIT_HIGH,
};
/// Current state
uint8_t adb_state;

// State information for transmitting a byte
/// Byte to transmit
uint8_t adb_tx_data;
/// Index of bit to transmit
uint8_t adb_tx_index;

ISR(TIMER0_COMP_VECTOR)
{
  switch(adb_state) {
  case ADB_STATE_TX_ATTN:
    adb_state = ADB_STATE_TX_SYNC;
    ADB_PORT = ADB_TX_HIGH;
    // Set up timer for 70us.
    TCCR0 = (TCCR0 & 0xF8) | 0x1; // prescalar = 8
    TCNT0 = 0;
    OCR0 = 70 / 0.5;
    break;
  case ADB_STATE_TX_SYNC:
  case ADB_STATE_TX_BIT_HIGH:
    if (adb_tx_index > 8) {
      adb_state = ADB_STATE_IDLE;
      ADB_PORT = ADB_TX_HIGH;
      TIMSK0 &= ~(_BV(1));
      break;
    }
    ADB_PORT = ADB_TX_LOW;
    adb_state = ADB_STATE_TX_BIT_LOW;
    // Set up timer for either 35us or 65us.
    TCNT0 = 0;
    if (adb_tx_index == 8) {
      OCR0 = 65 / 0.5;
    } else if ((adb_tx_data >> adb_tx_index) & 0x1 == 0) {
      OCR0 = 65 / 0.5;
    } else {
      OCR0 = 35 / 0.5;
    }
    break;
  case ADB_STATE_TX_BIT_LOW:
    ADB_PORT = ADB_TX_HIGH;
    adb_state = ADB_STATE_TX_BIT_HIGH;
    // Set up timer for either 65us or 35us.
    TCNT0 = 0;
    if (adb_tx_index == 8) {
      OCR0 = 35 / 0.5;
    } else if ((adb_tx_data >> adb_tx_index) & 0x1 == 0) {
      OCR0 = 35 / 0.5;
    } else {
      OCR0 = 65 / 0.5;
    }
    adb_tx_index++;
    break;
  default:
    break;
  }

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
  PORTA &= ~(_BV(1));
  DDRB = 0xff;

  // Construct command byte
  adb_tx_data = 0;
  adb_tx_data |= address << 4;
  adb_tx_data |= command << 2;
  adb_tx_data |= reg;
  adb_tx_index = 0;

  // Set output
  ADB_PORT = ADB_TX_LOW;
  // Kick off the timer for 800us
  TCCR0 = (TCCR0 & 0xF8) | 0x3; // prescalar = 64
  TCNT0 = 0;
  OCR0 = 800 / 4;
  // Set the state
  adb_state = ADB_STATE_TX_ATTN;

  PORTA |= _BV(1);
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

  PORTA &= ~(_BV(2));

  // Initialize resources
  DDRB = 0x00;
  adb_rx_len = 0;
  memset(adb_rx_buff, 0, 8*sizeof(uint8_t));

  // Begin a busy-wait loop to delay for up to 240us. If the data line drops
  // low during this time then discard the first bit and begin receiving
  // data.
  while(delay)
    {
      _delay_us(1.0);
      delay--;
      if (bit_is_clear(ADB_PIN, 2)) {
	receiving = 1;
	PORTA &= ~(_BV(3));
	while(bit_is_clear(ADB_PIN, 2)) {};
	while(bit_is_set(ADB_PIN, 2)) {};
	PORTA |= _BV(3);
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
      PORTA &= ~(_BV(4));
      while(bit_is_clear(ADB_PIN, 2)) {
	_delay_us(1.0);
	ticks++;
      }
      PORTA |= _BV(4);

      // Based on the length of the low portion of the bit we know if it's a
      // 0 or 1. A 1 will be 30us, and a 0 will be 45us.
      if (ticks > 37) {
	last_bit = 0;
      } else {
	last_bit = 1;
      }

      // Store the bit into the buffer. This stores the bits in increasing
      // bit position, meaning that the data is interpreted as being sent LSB
      // first. This may be an incorrect assumption.
      // TODO Figure out if data is MSB or LSB first (probably MSB).
      // TODO This will fail when the device sends 8B because of the stop bit
      uint8_t i = adb_rx_len / 8;
      /* assert(i <= 8); */
      adb_rx_buff[i] = (adb_rx_buff[i] << 1) | last_bit;
      adb_rx_len++;

      // Delay for the remaining time of the bit pulse. This should be 80us
      // less the length of the initial low pulse. We also monitor the data
      // line so we don't blow past the end of the bit and shave time off the
      // next one.
      int8_t remaining = 80 - ticks;
      ticks = 0;
      PORTA &= ~(_BV(5));
      while(bit_is_set(ADB_PIN, 2) && (ticks < remaining)) {
	_delay_us(1.0);
	ticks++;
      }
      PORTA |= _BV(5);

      // If the data line still hasn't gone low, and we've waited for the
      // next bit, then the device has stopped transmitting and we're done.
      // We verify that the last bit sent is indeed the "0" stop bit and
      // shave the bit off the bit count.
      if (ticks == remaining) {
	/* assert(last_bit == 0); */
	adb_rx_len = adb_rx_len - 1;
	receiving = 0;
      }
    }

  PORTA |= _BV(2);

  // Return 0 if we received data
  if (adb_rx_len)
    return 0;
  else
    return 1;
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
  //DDRC = 0xFF;

  // Reach steady state then reset devices
  // TODO this will probably have to change when USB is added.
  ADB_PORT = ADB_TX_HIGH;
  _delay_ms(1000.0);
  ADB_PORT = ADB_TX_LOW;
  _delay_ms(4.0);
  ADB_PORT = ADB_TX_HIGH;

  // Initialize to default keyboard address
  // keyboard: 0x2
  // mouse: 0x3
  last_device = 2;

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
  uint8_t poll_result;

  PORTA &= ~(_BV(0));

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

  PORTA |= _BV(0);

  return poll_result;
}
