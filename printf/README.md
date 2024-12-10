This is a thrid bit of "bare metal" code for the EBAZ4205

Tom Trebisky  1-8-2021

This is just the button demo with a printf function added.

It clicks away sampling the GPIO button pin a about 1 hz
and reports any changes -- so you see when you press the
button and again when you release it.

Note that the button is quite slow with the RC filter that is on
the board.  Not my doing.  It certainly avoids noise or bounce issues.
