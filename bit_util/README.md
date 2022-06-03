bit_util

6-3-2022  Tom Trebisky

Utilities to manipulate Xilinx bitstream files.

"bitread" is a C program that can read a Xilinx bitstream file.
It can:

1. Just list the header information
2. Extract a ".bin" file from the ".bit" file (-e option)
3. Generate C code (the -c option)

See the Makefile for example usage.

I learned from the openFPGALoader project, which deserves your attention.
I found a way to use that project to load a .bit file into the Zynq PL on
the Ebaz board (see my Makefile).

Someday I intend to create a program "bitload" that would do what
openFPGALoader does, communicating to the XVC protocol.
