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

    This keyboard has option and command keys instead of super and alt. This
    library will remap those accordingly.
*/

#include <stdlib.h>
#include <stdint.h>
#include <avr/pgmspace.h>

/// Represent a translation from ADB to USB or ascii
struct keycode_translation {
  unsigned char adb;
  unsigned char usb;
  char ascii;
};

/** \brief ADB to USB translation
 *
 * Maps ADB keycodes to USB HID values. See chapter 10 of the 
 * USB HID Usage Tables document.
 */
struct keycode_translation keycodes[] PROGMEM = {

  // <esc> 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
  {0x35, 41, ' '},
  {0x7a, 58, ' '},
  {0x78, 59, ' '},
  {0x63, 60, ' '},
  {0x76, 61, ' '},
  {0x60, 62, ' '},
  {0x61, 63, ' '},
  {0x62, 64, ' '},
  {0x64, 65, ' '},
  {0x65, 66, ' '},
  {0x6d, 67, ' '},
  {0x67, 68, ' '},
  {0x6f, 69, ' '},
  {0x69, 104, ' '},
  {0x6b, 105, ' '},
  {0x71, 106, ' '},

  // ~ 1 2 3 4 5 6 7 8 9 0 - + <del>
  {0x32, 53, '`'},
  {0x12, 30, '1'},
  {0x13, 31, '2'},
  {0x14, 32, '3'},
  {0x15, 33, '4'},
  {0x17, 34, '5'},
  {0x16, 35, '6'},
  {0x1a, 36, '7'},
  {0x1c, 37, '8'},
  {0x19, 38, '9'},
  {0x1d, 39, '0'},
  {0x1b, 45, '-'},
  {0x18, 46, '='},
  {0x33, 42, ' '},
  /* <tab> q w e r t y u i o p [ ] \ */
  {0x30, 43, ' '},
  {0x0c, 20, 'q'},
  {0x0d, 26, 'w'},
  {0x0e, 8, 'e'},
  {0x0f, 21, 'r'},
  {0x11, 23, 't'},
  {0x10, 28, 'y'},
  {0x20, 24, 'u'},
  {0x22, 12, 'i'},
  {0x1f, 18, 'o'},
  {0x23, 19, 'p'},
  {0x21, 47, '['},
  {0x1e, 48, ']'},
  {0x2a, 49, '\\'},
  // <cap> a s d f g h j k l ; ' <ret>
  {0x39, 57, ' '},
  {0x00, 4, 'a'},
  {0x01, 22, 's'},
  {0x02, 7, 'd'},
  {0x03, 9, 'f'},
  {0x05, 10, 'g'},
  {0x04, 11, 'h'},
  {0x26, 13, 'j'},
  {0x28, 14, 'k'},
  {0x25, 15, 'l'},
  {0x29, 51, ';'},
  {0x27, 52, '\''},
  {0x24, 40, ' '},
  // <shift> z x c v b n m , . / <shift>           
  // 38 (shift)
  {0x06, 29, 'z'},
  {0x07, 27, 'x'},
  {0x08, 06, 'c'},
  {0x09, 25, 'v'},
  {0x0b, 05, 'b'},
  {0x2d, 17, 'n'},
  {0x2e, 16, 'm'},
  {0x2b, 54, ','},
  {0x2f, 55, '.'},
  {0x2c, 56, '/'},
  // 38 (shift)
  // <ctrl> <o> <c> <space> <c> <o> <ctrl>        
  // 36 (ctrl)
  // 3a (opt -> super)
  // 37 (command -> alt)
  {0x31, 44, ' '},
  // 37 (command -> alt)
  // 3a (opt -> super)
  // 36 (ctrl)

  // <help> <hom> <pgup>
  {0x72, 117, ' '},
  {0x73, 74, ' '},
  {0x74, 75, ' '},
  // <del>  <end> <pgdn>
  {0x75, 76, ' '},
  {0x77, 77, ' '},
  {0x79, 78, ' '},
  
  // up arrow      
  {0x3e, 82, ' '},
  // left, down, right arrows
  {0x3b, 80, ' '},
  {0x3d, 81, ' '},
  // ??

  // <c> = / *
  {0x47, 83, ' '},
  {0x51, 103, '='},
  {0x4b, 84, '/'},
  {0x43, 85, '*'},
  // 7 8 9 -
  {0x59, 95, '7'},
  {0x5b, 96, '8'},
  {0x5c, 97, '9'},
  {0x4e, 86, '-'},
  // 4 5 6 +
  {0x56, 92, '4'},
  {0x57, 93, '5'},
  {0x58, 94, '6'},
  {0x45, 87, '+'},
  // 1 2 3 <ent>
  {0x53, 89, '1'},
  {0x54, 90, '2'},
  {0x55, 91, '3'},
  {0x4c, 88, ' '},
  // 0 .
  {0x52, 98, '0'},
  {0x41, 99, '.'},

  {0, 0, 0}
};

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
    uint8_t adb_code = keycode & 0x7f;

    // Modifier keys are handled separately.
    switch(adb_code) {
    case 0x38:
      kb_mod_shift = pressed;
      return 0;
    case 0x36:
      kb_mod_ctrl = pressed;
      return 0;
    case 0x3a: 
      kb_mod_opt = pressed;
      return 0;
    case 0x37: 
      kb_mod_com = pressed;
      return 0;
    }
    
    // Search for the translation for this keycode.
    uint8_t index = 0;
    while(pgm_read_byte(&keycodes[index].usb) != 0) {
      if (pgm_read_byte(&keycodes[index].adb) == adb_code) {
	kb_key = pgm_read_byte(&keycodes[index].usb);
      }
      index++;
    }

    // Zero out the value if the key was being released.
    if (!pressed) {
      kb_key = 0;
    }

    return 0;
}

/** \brief Return modifiers.
 *
 * Returns a byte representing the current set of pressed modifiers that should
 * be used in the keyboard HID report. The layout of this byte is:
 *
 * \verbatim
 * 0b00000000
 *   ||||||||_ left control
 *   |||||||__ left shift
 *   ||||||___ left alt (command)
 *   |||||____ left gui (option)
 *   ||||_____ right control
 *   |||______ right shift
 *   ||_______ right alt (command)
 *   |________ right gui (option)
 * \endverbatim
 *
 * Unfortunately the Apple Extended Keyboard II will return the same keycode
 * for both left and right keys. This function will only populate the left
 * modifier keys.
 *
 * @return uint8_t modifiers
 */
uint8_t kb_usbhid_modifiers()
{
  uint8_t mods = 0;

  mods |= kb_mod_ctrl;
  mods |= kb_mod_shift << 1;
  mods |= kb_mod_com << 2;
  mods |= kb_mod_opt << 3;

  return mods;
}

/** \brief Return current keys in USB representation.
 *
 * Returns an array of the currently pressed keys for use in an HID report.
 */
void kb_usbhid_keys(uint8_t *keys)
{
  // Right now I only support one key at a time, so this is easy.
  keys[0] = kb_key;

  return;
}

/** \brief Convert keycode to char
 *
 * Converts a keycode returned from polling the keyboard into a char. Currently
 * only supports keys with printable ASCII characters and does not handle the
 * shift key. Any unsupported keycodes will return ' '.
 * 
 * @param[in]   keycode value returned from keyboard
 * @return      char representation.
 */
char kb_dtoa(uint8_t keycode)
{
  uint8_t pressed = ~keycode & 0x80;
  uint8_t adb_code = keycode & 0x7f;

  if (!pressed) {
    return ' ';
  }

  // Search for a translation.
  uint8_t index = 0;
  while(pgm_read_byte(&keycodes[index].adb) != 0) {
    if (pgm_read_byte(&keycodes[index].adb) == adb_code) {
      return pgm_read_byte(&keycodes[index].ascii);
    }
    index++;
  }
  
  return ' ';
}

