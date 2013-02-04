ADBUSB Manual {#mainpage}
=============

ADBUSB is an adapter that turns an ADB keyboard into a USB HID
keyboard. I found an Apple Extended Keyboard II at the MIT fleamarket
a few years back and wanted to be able to use it for my day-to-day
programming. I looked around at the various products that turned ADB
to USB, and decided to roll my own (as any computer engineer
should). This project is the fruit of those efforts and I have strived
to be as verbose and thorough as possible. Everything you would need
to build your own, including source code, schematics, and parts lists,
is included.

The latest source is always available at https://github.com/devrintalen/adbusb

Features:
* Self-powered
* Translates ADB to USB
* Works with the complete ADB keycode set (all keys, function keys, etc. are supported)
* No drivers to install (HID-compliant)
* Tested

Limitations:
* Only supports one keyboard connection (no daisy-chaining, no mice, etc.)
* Created for AVR microcontrollers only

This project makes use of the [V-USB] open-source USB driver.

This manual is divided into the following sections:
* [Installation](@ref install)
* [Schematics and hardware](@ref circuit)
* [Apple Desktop Bus](@ref adb)
* [References and further reading](@ref references)

[V-USB]: http://www.obdev.at/vusb/
