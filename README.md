EBAZ4205 Bitcoin miner board

These boards became available at a very attractive price in late 2020.
They contain a Xilinx ZYNQ XC7Z010 chip, which contains a dual core ARM
(Cortex-A9) that runs at 667 Mhz along with the usual peripherals, and
above all else a substantial FPGA.

This repository contains my research efforts with the board I have.
Ultimately I will be doing bare metal programming and things of
that sort.

Most of my notes are on my website:

http://cholla.mmto.org/ebaz4205

Here are the projects you will find here:

1. master - Ebaz Master constraint XDC file for Vivado

1. setup - my process to configure U-Boot for network tftp booting
1. fish - why "hello world" for a first program?
1. button - watch for the button connected to the gpio
1. printf - I add a little printf to the button demo
1. timer - basic interrupts
1. amp - asymmetric multiprocessing (get second core running)

1. bit_util - bitstream utilities
1. blink1 - blink (test PL download)
1. blink2 - blink (make an AXI gpio LED gadget and AXI gpio driver)
