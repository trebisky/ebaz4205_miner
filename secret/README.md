
Python scripts (and some other stuff) to play with
the "secret" uart loader option in the Zynq bootrom.

I am testing against an Antminer S9 board which has the
Zynq XC7Z010-CLG400A chip.  On this board the serial console
is on the correct pins and there are handy jumpers for all
of the boot mode options, making all of this relatively easy.

1. 02-zynq-uart.py - original script by "404"
1. dumper.py - original script by "404"

1. e2501.py - provoke error 2501
1. e2502.py - provoke error 2502
1. e2503.py - provoke error 2503

1. nand.dump - dump the FSBL from NAND and examine what the
bootrom header looks like in something that actually boots.

Tom Trebisky  5-22-2024
