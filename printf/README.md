This is my second bit of "bare metal" code for the EBAZ4205

Tom Trebisky  1-8-2021

This program prints reads gpio input bank 0 and watches
for the value to change.  The idea is to print (via the serial
port) when I push and release the button.

It works!  The code takes every manner of short cut rather than
designing general interfaces, but at this stage we are just doing
simple proof of concept experiments.
