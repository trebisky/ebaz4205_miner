Get the second core on the Zynq-7000 doing something.

I am continuing my experiments with my EBAZ4205 board.

This now works!  Having the first core execute the SEV
instruction to wake up the second was the key.

The bulk of this is the "timer" project, which got
interrupts working for a timer and uart receive.
Since I just copied that and started adding stuff,
there is a fair bit of "extra stuff" that isn't
directly related to starting up a second core.
I have tried to put all the interesting new stuff
in cores.c
