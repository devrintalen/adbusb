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

/** \file uart.c
    \brief UART driver.
*/

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

/// Initialize driver resources
void uart_init(void)
{
  UBRRL = 103; // 9600 baud
  UCSRB = _BV(TXEN); // enable tx
}

/// Send a single character on the UART.
int uart_putchar(char c, FILE *stream)
{
  if (c == '\n')
    uart_putchar('\r', stream);

  loop_until_bit_is_set(UCSRA, UDRE);
  UDR = c;

  return 0;
}
