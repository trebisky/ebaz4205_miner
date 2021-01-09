This is my first bit of "bare metal" code for the EBAZ4205
It is loaded by U-boot and so is not truly "bare metal" and
in fact it relies on the uart initialization already done
by U-boot.  The goal is to get something (anything!) to run
with the least amount of code possible!

Tom Trebisky  1-8-2021

This program prints "fish" endlessly on the serial port.
I have had enough with "hello world" and figured that I
would just do something different.

The starting point for this was my Orange Pi Github
archive, project "h5_hello".
