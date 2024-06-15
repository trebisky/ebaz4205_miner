blink2 project

6-3-2022, 6-6-2022 6-15-2024

Here we have a bitstream for the PL that implements a GPIO
driven special blink function that can be controlled by
the GPIO.  The blinking is entirely autonomous once it is
started.

As soon as a character is typed, blinking stops.

This was copied from "blink1", which was copied from "timer".

The DMA loading of the PL is very fast.

This works with bit_read generating C source
from the bitstream file.
