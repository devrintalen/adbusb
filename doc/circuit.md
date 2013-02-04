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
pull-up resistor between the data line and Vcc.

Connecting to USB
-----------------

The USB hardware requirements are dictated by the V-USB driver.
