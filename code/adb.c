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
  ADB_STATE_RX_HIGH,
  ADB_STATE_HOLD
};
/**
 * Current state. This ADB driver is interrupt-based and requires a
 * state machine to keep track of what's going on. A description of
 * each of the states and their meanings is at adb_states.
 */
uint8_t adb_state;

// State information for transmitting a byte
/**
 * Byte to transmit. An ADB command is 8b sent MSB first (followed by
 * a stop bit). This value will remain unchanged during the transmit
 * code and adb_tx_index will be used to reference the bit that is
 * currently being sent.
 */
uint8_t adb_tx_data;

/**
 * Index of bit to transmit. References the bit position in adb_tx_data
 * that is currently being sent. This starts at 7 and decrements to 0.
 * Because ADB requires a stop bit, this variable is decremented past 0
 * to -1 and -2. The meanings of each of the possible values are:
 *
 * - 7-0: normal.
 * - -1: Sending the stop bit (same as 0).
 * - -2: Stop before sending anything, will transition to RX code
 *       when this value is hit.
 */
int8_t adb_tx_index;

// State information for receiving data
/// Received data
uint8_t adb_rx_data[9];
/// Number of bits received
uint8_t adb_rx_count;


/**
 * Timer0 compare interrupt. Triggered when timer0 matches the compare
 * value. This is used to make the ADB code send out the next bit.
 */
ISR(TIMER0_COMP_vect, ISR_NOBLOCK)
{
  TCNT0 = 0;

  PORTA &= ~(_BV(0));

  switch (adb_state) {

  case ADB_STATE_TX_ATTN:
    // Just finished sending the ATTN pulse.
    adb_state = ADB_STATE_TX_SYNC;
    ADB_PORT = ADB_TX_1;
    // Set up timer for 70us.
    TCCR0 = 0xa;
    OCR0 = 70 / 0.5;
    break;

  case ADB_STATE_TX_SYNC:
    // Just finished the SYNC pulse, which is the same as...
  case ADB_STATE_TX_BIT_HIGH:
    // Just finished the high part of a bit.
    if (adb_tx_index == -2) {
      adb_state = ADB_STATE_RX_WAIT;
      // Set up port to receive data
      ADB_PORT = ADB_TX_1;
      DDRB = 0x00;
      // Enable INT2 to catch a falling edge
      GICR &= ~(_BV(5));
      MCUCSR &= ~(_BV(6));
      GIFR |= _BV(5);
      GICR |= _BV(5);
      // Start counting time, up to 240us
      TCCR0 = 0xb;
      OCR0 = 240 / 4;
      break;
    }
    ADB_PORT = ADB_TX_0;
    adb_state = ADB_STATE_TX_BIT_LOW;
    // Set up timer for either 35us or 65us.
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
    if (adb_tx_index == -1) {
      OCR0 = 35 / 0.5;
    } else if (((adb_tx_data >> adb_tx_index) & 0x1) == 0) {
      OCR0 = 35 / 0.5;
    } else {
      OCR0 = 65 / 0.5;
    }
    adb_tx_index--;
    break;

  case ADB_STATE_RX_LOW:
    // About 128us have elapsed since the last bit received had started.
    // The ADB device has stopped sending data and we need to stop
    // receiving data.
    TIMSK &= ~(_BV(1)); // disable timer interrupt
    // Disable INT2
    GICR &= ~(_BV(5));
    PORTA |= _BV(2);
    // All done!
    adb_state = ADB_STATE_HOLD;
    break;

  case ADB_STATE_RX_WAIT:
    // 240us have elapsed since the stop bit. If an external interrupt
    // had fired by this point the state would have been modified and
    // we wouldn't get here. Re-initialize everything.
    TIMSK &= ~(_BV(1)); // disable timer interrupt
    // Disable INT2
    GICR &= ~(_BV(5));
    // All done!
    adb_state = ADB_STATE_IDLE;
    break;

  default:
    break;
  }

  PORTA |= _BV(0);

  return;
}

/**
 * External interrupt on ADB pin. Triggered when an ADB device starts 
 * transmitting data to the processor.
 */
ISR(INT2_vect, ISR_NOBLOCK) {
  uint8_t adb_rx_low_duration = TCNT0;
  uint8_t adb_rx_bit;

  GICR &= ~(_BV(5));
  TCNT0 = 0;

  PORTA &= ~(_BV(1));

  switch (adb_state) {

  case ADB_STATE_RX_WAIT:
    TCCR0 = 0xa;
    OCR0 = 220;
    PORTA &= ~(_BV(2));
    // Purposefully fall through to the next state...

  case ADB_STATE_RX_LOW:
    adb_state = ADB_STATE_RX_HIGH;
    // Enable INT2 to catch a rising edge
    MCUCSR |= _BV(6);
    break;

  case ADB_STATE_RX_HIGH:
    // Record the bit.
    if (adb_rx_low_duration > (40 * 2)) {
      adb_rx_bit = 0;
    } else {
      adb_rx_bit = 1;
    }
    adb_rx_data[adb_rx_count / 8] |= adb_rx_bit << (7 - (adb_rx_count % 8));
    adb_rx_count++;
    adb_state = ADB_STATE_RX_LOW;
    // Enable INT2 to catch a falling edge
    MCUCSR &= ~(_BV(6));
    break;

  default:
    break;
  }
  
  // Re-enable the external interrupt.
  GIFR |= _BV(5);
  GICR |= _BV(5);
    
  PORTA |= _BV(1);

  return;
}


/**
 * Initialize resources. This routine initializes the microprocesser 
 * resources used by the ADB code and performs the ADB bringup sequence. 
 * This consists of:
 *
 * -# Raise the line and remain stable for 1s.
 * -# Perform reset pulse for 4ms (spec states 3ms, but actual Mac II
 *    hardware will do 4ms).
 * -# Raise line.
 *
 * In addition to setting the ADB processor state interrupts will be enabled.
 *
 * @return 0 for success.
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

  return 0;
}


/**
 * Send a command packet and receive data if sent. Constructs a command
 * packet and sent according to the ADB specification:
 *
 * -# Assert attention signal (800us).
 * -# Assert sync signal (70us).
 * -# Send command byte (8 * 100us).
 * -# Send stop bit (100us).
 * -# Release line.
 *
 * After sending the command packet, this will wait for data to be 
 * returned. If the device begins sending data then this will begin 
 * recording it. The specification states that we must wait between
 * 160us and 240us for the device to start. A response packet looks like:
 *
 * -# Start bit (1)
 * -# Two to eight bytes of data, sent in order of 0 to 7. Each byte is
 *    sent MSB first.
 * -# Stop bit (0)
 *
 * Returned data is stored in adb_rx_data. The number of bits received
 * is available in adb_rx_count.
 *
 * This code uses timer0 and INT2 to make this call non-blocking.
 * Once the attention signal has been started this call will return.
 * Successive calls will return non-zero status until the state machine
 * reaches idle again.
 *
 * @param[in]  address Device address.
 * @param[in]  command Command to send.
 * @param[in]  reg     Register to read/write.
 * @return     0 for success.
 */
int8_t adb_command(uint8_t address, uint8_t command, uint8_t reg)
{
  if (adb_state != ADB_STATE_IDLE) {
    return 1;
  }

  // Prepare port to output
  DDRB = 0xff;

  // Construct command byte
  adb_tx_data = 0;
  adb_tx_data |= address << 4;
  adb_tx_data |= command << 2;
  adb_tx_data |= reg;
  adb_tx_index = 7; // data is sent MSB first

  // Prepare to receive data
  adb_rx_count = 0;
  memset((void *)adb_rx_data, 0, 9 * sizeof(uint8_t));

  // Start the state machine
  adb_state = ADB_STATE_TX_ATTN;
  ADB_PORT = ADB_TX_0;
  // Kick off the timer for 800us
  TCCR0 = 0xb;
  TCNT0 = 0;
  OCR0 = 800 / 4;
  TIMSK |= _BV(1);

  return 0;
}


/**
 * Read received data. The ADB state machine will not send
 * another command while there is data waiting in the buffer.
 * This copies the ADB data and bit count and resets the state,
 * allowing another command to be sent.
 *
 * @param[in] *len Pointer to store the bit count.
 * @param[in] buff Pointer to a uint8_t[8] buffer.
 * @return         0 if data was copied, 1 otherwise.
 */
int8_t adb_read_data(uint8_t *len, uint8_t *buff)
{
  uint8_t i;

  // First check to make sure we have received data.
  if (adb_state != ADB_STATE_HOLD) {
    return 1;
  }
  
  // Remove the start and stop bits from the data by shifting all
  // eight bytes left by one bit.
  for(i=0; i<8; i++) {
    adb_rx_data[i] = (adb_rx_data[i] << 1) | ((adb_rx_data[i + 1] & 0x80) >> 7);
  }
  adb_rx_count = adb_rx_count - 2;

  // Copy data.
  *len = adb_rx_count;
  memcpy((void *)buff, (void *)adb_rx_data, 8 * sizeof(uint8_t));

  // Reset state machine.
  adb_state = ADB_STATE_IDLE;

  return 0;
}
