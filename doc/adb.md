Apple Desktop Bus {#adb}
=================

Apple Desktop Bus (abbreviated ADB) is a host-controlled synchronous
protocol developed for the peripherals for the original Apple
computer. You'll find it on old Apple peripherals, including the
venerable Extended Keyboard II and those crappy one-button mice.

To get a more detailed overview, and for the history, read the
[Wikipedia page on ADB][wiki].

Physical
--------

ADB devices are connected by a 4-pin mini-DIN connector. It's hard to
find old ADB cables, but fortunately an s-video cable is the exact
same thing (and much more plentiful). The layout of the pins, looking
from the front, is:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  4 3         3 4     1 | Data
 2   1       1   2    2 | Power switch
   =           =      3 | +5V
 Female       Male    4 | Gnd
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Protocol
--------

All requests are made by the host and are in bytes. Data is sent MSB
first, in order of lowest byte to highest byte. Every bit is a
combination of a low pulse and a high pulse for 100us, where the width
of each determines which it is.

* 0 is 65us low, 35us high.
* 1 is 35us low, 65us high.
* Reset is a low signal for at least 1s.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   _      __        _    ____
0:  \____/  \_   1:  \__/    \_
      65  35          35  65
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A request byte sent from the host is constructed in this way. The
address that is used can be any four bit value, but after reset
keyboards will default to address `0x2` and mice to `0x3`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 7        4  3  2  1  0
+----------+-----+-----+
|   Addr   | Cmd | Reg |
+----------+-----+-----+
                |      \_ 0: primary
                |         1: n/a
                |         2: n/a
                |         3: device ID
                |
                 \_______ 0: flush
                          1: n/a
                          2: listen
                          3: talk
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The protocol to initialize the bus is:

1. Host signals reset.
2. Host sends talk commands to addresses `0x2` and `0x3` for register `0x3` (device ID).
3. Device responds and moves to higher address (chosen randomly).
4. Host tells device to move from new address to an address it chooses.

After initialization the flow is:

1. Attention signal (low for 800us).
2. Sync signal (high for 70us).
3. Command packet
   * 8 bits (100us each)
   * Stop bit (same as a 0)
4. Tlt signal (stop-to-start time, high for 160 to 240us).
5. Data packet
   * 2-8 bytes

Devices ask for attention with a service request (Srq) signal. This is
a low signal for 300us. An Srq can only be sent by a device during the
stop bit cell time if the request is not for it (to a different
address).

The default active device is \c 0x3. The host should continuously poll
the last active device (that asserted Srq). The device will only
respond if it has data to send.

Implementation
--------------

Since ADBUSB only supports one physical connection to one keyboard it
takes a few shortcuts with the protocol to simplify the
implementation. For starters, the flow to initialize devices to unique
addresses is not taken. The full flow, along with any assumptions, is
documented below:

1. Host signals reset for 1s.
2. Host delays for 4ms.
3. Host begins an infinite loop of this sequence:
   1. Attention signal (low for 800us).
   2. Sync signal (high for 70us).
   3. Command packet
      * 8 bits (100us each)
      * Stop bit (same as a 0)
   4. Tlt signal (stop-to-start time, high for 160 to 240us).
   5. Data packet
      * 2-8 bytes

[wiki]: http://en.wikipedia.org/wiki/Apple_Desktop_Bus
