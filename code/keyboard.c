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

/** \file keyboard.c
    \brief ADB keyboard library.

    Defines routines to translate ADB keyboard data. The Apple Extended
    Keyboard II (M3501) defines unique keycodes for each button press and
    release. Modifiers are not encoded in the keycode and so must be tracked
    in software.

    When a key is pressed, bit 7 is 0. When it is released, bit 7 is 1.

    I have determined the keycode for each key on the keyboard below:

    \verbinclude keyboard_layout.rst
*/

#include <stdlib.h>
#include <stdint.h>


/// Shift key modifier flag
uint8_t kb_mod_shift;
/// Control key modifier flag
uint8_t kb_mod_ctrl;
/// Opt key modifier flag
uint8_t kb_mod_opt;
/// Command key modifier flag
uint8_t kb_mod_com;

/// Capslock flag
uint8_t kb_tog_capslock;

/// Current key
uint8_t kb_key;

/** \brief Register a keypress
 *
 * Sets internal keyboard state according to a keycode returned from the ADB
 * keyboard. Tracks modifier keys and regular keys.
 *
 * @param[in]   keycode 8b value returned from keyboard.
 * @return      0 for success.
 */
uint8_t kb_register(uint8_t keycode)
{
    // The top bit of the keycode tells us whether a key was pressed or
    // released. It is 0 when pressed and 1 when released.
    uint8_t pressed = ~keycode && 0x80;

    // Evaluate the rest of the keycode.
    switch(keycode & 0x7f)
    {
        // Modifier keys
        case 0x38: kb_mod_shift = pressed; break;
        case 0x36: kb_mod_ctrl = pressed; break;
        case 0x3a: kb_mod_opt = pressed; break;
        case 0x37: kb_mod_com = pressed; break;

        // Keys
        case 0x00: kb_key = USB_KEY_A; break;
        case 0x0b: kb_key = USB_KEY_B; break;
        case 0x08: kb_key = USB_KEY_C; break;
        case 0x02: kb_key = USB_KEY_D; break;
        case 0x0e: kb_key = USB_KEY_E; break;
        case 0x03: kb_key = USB_KEY_F; break;
        case 0x05: kb_key = USB_KEY_G; break;
        case 0x04: kb_key = USB_KEY_H; break;
        case 0x22: kb_key = USB_KEY_I; break;
        case 0x26: kb_key = USB_KEY_J; break;
        case 0x28: kb_key = USB_KEY_K; break;
        case 0x25: kb_key = USB_KEY_L; break;
        case 0x2e: kb_key = USB_KEY_M; break;
        case 0x2d: kb_key = USB_KEY_N; break;
        case 0x1f: kb_key = USB_KEY_O; break;
        case 0x23: kb_key = USB_KEY_P; break;
        case 0x0c: kb_key = USB_KEY_Q; break;
        case 0x0f: kb_key = USB_KEY_R; break;
        case 0x01: kb_key = USB_KEY_S; break;
        case 0x11: kb_key = USB_KEY_T; break;
        case 0x20: kb_key = USB_KEY_U; break;
        case 0x09: kb_key = USB_KEY_V; break;
        case 0x0d: kb_key = USB_KEY_W; break;
        case 0x07: kb_key = USB_KEY_X; break;
        case 0x10: kb_key = USB_KEY_Y; break;
        case 0x06: kb_key = USB_KEY_Z; break;

        case 0x12: kb_key = USB_KEY_1; break;
        case 0x13: kb_key = USB_KEY_2; break;
        case 0x14: kb_key = USB_KEY_3; break;
        case 0x15: kb_key = USB_KEY_4; break;
        case 0x17: kb_key = USB_KEY_5; break;
        case 0x16: kb_key = USB_KEY_6; break;
        case 0x1a: kb_key = USB_KEY_7; break;
        case 0x1c: kb_key = USB_KEY_8; break;
        case 0x19: kb_key = USB_KEY_9; break;
        case 0x1d: kb_key = USB_KEY_0; break;

        default: kb_key = 0;
    }

    // Zero out the value if the key was being released.
    kb_key = kb_key && pressed;

    return 0;
}

/** \brief Get an HID report
 *
 * Creates an HID report according to the internal keyboard state.
 *
 * @param[in]   buffer  Pointer to a 2x8b array.
 * @return      0 for success.   
 */
uint8_t kb_get_hid(uint8_t *buffer)
{
    // Set the lower byte of the buffer to the current key.
    buffer[0] = kb_key;

    // Set the high byte of the buffer to the current set of modifiers.
    buffer[1] = 0;
    buffer[1] |= kb_mod_shift & USB_MOD_SHIFT_LEFT;
    buffer[1] |= kb_mod_ctrl & USB_MOD_CONTROL_LEFT;
    buffer[1] |= kb_mod_opt & USB_MOD_GUI_LEFT;
    buffer[1] |= kb_mod_com & USB_MOD_ALT_LEFT;

    return 0;
}

/** \brief Convert keycode to char
 *
 * Converts a keycode returned from polling the keyboard into a char. Currently
 * only supports printable ASCII characters that don't need the shift key.
 * 
 * @param[in]   d 8b value to decode.
 * @return      char representation.
 */
char kb_dtoa(uint8_t d)
{
    switch(d)
    {
        // Row 1
        case 0x32: return '~'; break;
        case 0x12: return '1'; break;
        case 0x13: return '2'; break;
        case 0x14: return '3'; break;
        case 0x15: return '4'; break;
        case 0x17: return '5'; break;
        case 0x16: return '6'; break;
        case 0x1a: return '7'; break;
        case 0x1c: return '8'; break;
        case 0x19: return '9'; break;
        case 0x1d: return '0'; break;
        case 0x1b: return '-'; break;
        case 0x18: return '+'; break;
        case 0x33: return '\b'; break;

        // Row 2
        case 0x30: return '\t'; break;
        case 0x0c: return 'q'; break;
        case 0x0d: return 'w'; break;
        case 0x0e: return 'e'; break;
        case 0x0f: return 'r'; break;
        case 0x11: return 't'; break;
        case 0x10: return 'y'; break;
        case 0x20: return 'u'; break;
        case 0x22: return 'i'; break;
        case 0x1f: return 'o'; break;
        case 0x23: return 'p'; break;
        case 0x21: return '['; break;
        case 0x1e: return ']'; break;
        case 0x2a: return '\\'; break;

        // Row 3
        case 0x00: return 'a'; break;
        case 0x01: return 's'; break;
        case 0x02: return 'd'; break;
        case 0x03: return 'f'; break;
        case 0x05: return 'g'; break;
        case 0x04: return 'h'; break;
        case 0x26: return 'j'; break;
        case 0x28: return 'k'; break;
        case 0x25: return 'l'; break;
        case 0x29: return ';'; break;
        case 0x27: return '\''; break;
        case 0x24: return '\n'; break;

        // Row 4
        case 0x06: return 'z'; break;
        case 0x07: return 'x'; break;
        case 0x08: return 'c'; break;
        case 0x09: return 'v'; break;
        case 0x0b: return 'b'; break;
        case 0x2d: return 'n'; break;
        case 0x2e: return 'm'; break;
        case 0x2b: return ','; break;
        case 0x2f: return '.'; break;
        case 0x2c: return '/'; break;

        // Row 5
        case 0x31: return ' '; break;

        default: break;
    }

    return '!';
}

