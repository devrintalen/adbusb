Schematics and Hardware {#circuit}
=======================

This project doesn't require much in the way of hardware. With a
development board like an STK500, connecting to an ADB device only
requires a pull-up resistor. Connecting to USB is slightly more
complicated, but still relatively easy.

Connecting to ADB
-----------------

Two pins are needed: one for the data line and one for reset. The two
others should be connected to +5V and ground. There needs to be a 1.5K
pull-up resistor between the data line and Vcc. A sample circuit looks like:

![Sample ADB schematic](../sample_adb.png)

Connecting to USB
-----------------

The USB hardware requirements are dictated by the V-USB driver. You should always read up on the latest [V-USB hardware requirements][vusb_hw], but as of this writing one of the recommended solutions was this:

![Sample V-USB schematic](../with-zener.png)

This setup is slightly more complicated than just stepping down the 5V supply from USB and running everything at 3.6V (which is what the USB data lines require), but ADB hardware requires a 5V supply for power and data. So keep the 5V supply and instead just step down the USB data lines.

Reference Implementation
------------------------

Putting this all together, we have the reference ADBUSB implementation, available under the `schematics/` directory:

![Reference implementation](../circuit_mega32.png)

The ADB hardware is in the bottom left, and the USB hardware in the upper right. The rest is standard fare for hooking up an AVR microcontroller. In the upper left is the programming header. The (very) bottom left is the crystal oscillator circuit. The four capacitors on the right side are meant to be put between the 5V supply and the Vcc inputs on the chip. See the [Atmel AVR042: AVR Hardware Design Considerations][1] manual for more information on how to hook up AVR microcontrollers.


[1]: http://www.atmel.com/Images/doc2521.pdf
[vusb_hw]: http://vusb.wikidot.com/hardware
