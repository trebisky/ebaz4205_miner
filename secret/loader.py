#!/bin/env python3

# Derived from e2503.py

# On my Fedora 40 system, I needed:
# dnf install python3-pyserial

from struct import pack as p
from struct import unpack as up
import serial
import time
import sys

# =================================================

class Zynq () :

    def __init__ ( self ) :
        ser = serial.Serial(timeout=0.5)
        ser.port = "/dev/ttyUSB0"
        ser.baudrate = 115200
        ser.open()
        self.ser = ser
        self.size = 0

    def wait ( self ) :
        print ( 'Waiting for XLNX-ZYNQ' )
        while self.ser.read(1) != b'X':
            continue
        assert self.ser.read(8) == b'LNX-ZYNQ'
        print ( "Got the XLNX-ZYNQ" );

    def show ( self, num, n ) :
        if num > 0 :
            print ( "Bytes waiting:", num, n );
        else :
            print ( "Bytes waiting:", n );
        msg = self.ser.read (n )
        print ( msg )
        #print ( str(msg, encoding='utf-8') )
        #print ( str(msg, encoding="ISO-8859-1" ) )

    def e2501 ( self ) :
        ser = self.ser
        num = 0
        while True:
            time.sleep(0.01)
            num += 1
            ser.write(b"X")
            n = ser.in_waiting
            if n != 0 :
                self.show ( num, n )
            if num > 500 :
                break

    # Any size greater than 0x3_0000
    # should trigger the 2502 error
    def e2502 ( self ) :
        ser = self.ser

        uart1 = 0
        uart2 = 0
        b_uart1 = uart1.to_bytes(4, 'little')
        b_uart2 = uart2.to_bytes(4, 'little')

        size = 0x3_3333
        b_size = size.to_bytes(4, 'little')

        ser.write ( b"BAUD" )
        ser.write ( b_uart1 )
        ser.write ( b_uart2 )
        ser.write ( b_size )

        # This delay is essential
        time.sleep(0.01)

        n = ser.in_waiting
        if n != 0 :
            self.show ( 0, n )
        else :
            print ( "Trouble" )

    def e2503 ( self ) :
        ser = self.ser

        # For this test we need real values here.
        # Once the bootrom gets a valid size it
        # will reinitialize the uart, then it will
        # read the checksum.
        uart1 = 0x11    # baud generator
        uart2 = 0x6
        b_uart1 = uart1.to_bytes(4, 'little')
        b_uart2 = uart2.to_bytes(4, 'little')

        size = 64
        b_size = size.to_bytes(4, 'little')

        # when we set cksum = 0 (skip checksum check) we get:
        # b'\r\n\r\nDec  1 2012 - 01:16:55\r\n3\r\nOutputStatus = 0x200A\r\xfe'
        # As below, this swallows 64 bytes, but it passes them along to
        # other layers, and they don't like the junk they get.
        # odds are they are trying to validate the bootheader, which fails.
        # cksum = 0
        #
        # The following gives us a dandy 0x2503 error.
        # Bytes waiting: 64 54
        # b'\r\n\r\nDec  1 2012 - 01:16:55\r\n3\r\nOutputStatus = 0x2503\r\xfe'
        # So ....
        # It swallowed the checksum, then 64 more bytes
        cksum = 0xdeadbeef
        b_cksum = cksum.to_bytes(4, 'little')

        ser.write ( b"BAUD" )
        ser.write ( b_uart1 )
        ser.write ( b_uart2 )
        ser.write ( b_size )

        ser.write ( b_cksum )

        time.sleep(0.01)

        num = 0
        for _ in range(200) :
            ser.write(b"X")
            num += 1
            time.sleep(0.01)
            n = ser.in_waiting
            if n != 0 :
                break

        if n != 0 :
            self.show ( num, n )
        else :
            print ( "Trouble" )

    def read_image ( self ) :
        binfile = "lots.bin"

        with open ( binfile, mode='rb') as file:
            self.image = file.read()
            self.size = len ( self.image )

        print ( "Binary image read: ", self.size, " bytes" )

    # Calculate header checksum
    def hdrchksum ( self, data ):
        chk = 0
        for i in range(0, len(data), 4):
            chk += up("<I", data[i:i+4])[0]
            chk &= 0xFFFF_FFFF
        return chk

    # This is copied from the code by "404" with my comments added
    # The header is described on pages 171 etc. in the TRM
    def mk_header ( self, im_size ) :
        # First 8 words that would be interrupt vectors
        # if we were doing execute in place (but we aren't)
        hdr  = p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)

        # 0x20 - width detect, always these magic bytes
        # that the bootrom uses to sort out how the qSPI
        # ROM is connected -- if one is being used.
        hdr += p("<I", 0xaa995566)

        # 0x24 - "image identification"
        # must always be these 4 magic characters
        hdr += b'XNLX'

        # 0x28 encryption status + misc
        # Anything other than 0xA5C3C5A3 or 0x3A5C3C5A
        # means we are not encrypted (so this might as well be zero)
        hdr += p("<I", 0)

        # 0x2c - stores bootrom version, but is ignored and could be anything.
        hdr += p("<I", 0x01010000)

        # 0x30 - source offset
        # :D ('source offset' - why yes, I'm like to boot the bootrom!)
        # hdr += p("<I", 0x1_0000_0000-0x40000)
        # Offset in bytes from the beginning of the bootrom header
        #  to where the FSBL (User code) starts.
        # Must be on a 64 byte boundary (0, 0x40, 0x80, 0xc0)
        #hdr += p("<I", im_offset)
        hdr += p("<I", 0x900)

        # 0x34 - length of image in bytes to go to OCM
        # must be <= 3*64K = 192K
        #hdr += p("<I", 0x2_0000)
        hdr += p("<I", im_size)

        # 0x38 - load address, I will always use 0
        hdr += p("<I", 0)

        # 0x3c = start address
        #hdr += p("<I", 0x0FCB4) -- spin loop in ROM
        hdr += p("<I", 0)

        # 0x40 - total image length to copy to OCM
        # The same as 0x34 for the unencrypted case
        #hdr += p("<I", 0x010014)
        hdr += p("<I", im_size)

        # 0x44 - qSPI config word (always 1)
        hdr += p("<I", 1)

        # 0x48 - header checksum (over 0x20 to 0x44)
        # sum the words, then invert the result
        hdr += p("<I", 0xffff_ffff - self.hdrchksum(hdr[0x20:]))

        # 0x4c to 0x97 - 19 words of "user defined"
        # stuff that the bootrom ignores.
        for _ in range(19):
            hdr += p("<I", 0)

        # 0x98 -- boot header table offset
        # pointer to image header table
        # who knows what this is all about.
        hdr += p("<I", 0x8c0)

        # 0x9c -- qSPI config word
        # pointer to partition header table
        # who knows what this is all about.
        hdr += p("<I", 0x8c0)

        # 0xa0 to 0x89c - register initialization
        # 512 words as 256 pairs
        # address, data
        # This could be used to instruct the bootrom
        # to initialize device registers.
        # An address of 0xffff_ffff ends the list
        for _ in range(0x100):
            hdr += p("<II", 0xffff_ffff, 0)

        return hdr

    def load ( self ) :
        ser = self.ser

        # This is "no change", i.e.
        # 115200 baud, no parity, 8 1
        uart1 = 0x11    # baud generator
        uart2 = 0x6
        b_uart1 = uart1.to_bytes(4, 'little')
        b_uart2 = uart2.to_bytes(4, 'little')

        size = self.size
        b_size = size.to_bytes(4, 'little')

        # Tell it to skip the checksum check
        cksum = 0
        b_cksum = cksum.to_bytes(4, 'little')

        ser.write ( b"BAUD" )
        ser.write ( b_uart1 )
        ser.write ( b_uart2 )
        ser.write ( b_size )

        ser.write ( b_cksum )

        time.sleep(0.01)

        header = self.mk_header ( self.size )
        ser.write ( header )

        ser.write ( self.image )

    def listen ( self ) :
        num = 0
        for _ in range(20) :
            num += 1
            time.sleep(0.1)
            n = self.ser.in_waiting
            if n != 0 :
                break

        if n != 0 :
            self.show ( num, n )
        else :
            print ( "Nothing" )

test = Zynq ()
#test.wait ()
#test.e2501 ()
#test.e2502 ()
#test.e2503 ()
test.read_image ()
test.wait ()
test.load ()
test.listen ()

print ( "Done" )

# THE END
