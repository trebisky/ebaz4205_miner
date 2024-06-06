#!/bin/env python3

# Derived from e2503.py

# On my Fedora 40 system, I needed:
# dnf install python3-pyserial

from struct import pack as p
from struct import unpack as up
import serial
import time
import sys

# Every once in a while I see this:
# To my eye, this looks like a bug in pyserial ...
# 
# Waiting for XLNX-ZYNQ
# Traceback (most recent call last):
#   File "/u1/Projects/FPGA/Ebaz/Github/secret/./loader.py", line 511, in <module>
#     test.wait ()
#   File "/u1/Projects/FPGA/Ebaz/Github/secret/./loader.py", line 30, in wait
#     while self.ser.read(1) != b'X':
#           ^^^^^^^^^^^^^^^^
#   File "/usr/lib/python3.12/site-packages/serial/serialposix.py", line 595, in read
#     raise SerialException(
# serial.serialutil.SerialException: device reports readiness to read but returned no data (device disconnected or multiple access on port?)


# =================================================

class Zynq () :

    def __init__ ( self ) :
        ser = serial.Serial(timeout=0.5)
        ser.port = "/dev/ttyUSB0"
        ser.baudrate = 115200
        ser.open()

        self.ser = ser
        self.size = 0
        self.sum = None

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

    def read_image ( self, binfile ) :
        try:
            with open ( binfile, mode='rb') as file:
                self.image = file.read()
                self.size = len ( self.image )
        except IOError:
            print ( "Trouble reading: ", binfile )
            exit ()

        print ( "Binary image read: ", self.size, " bytes" )

    # Calculate header checksum
    # (word by word)
    def hdrchksum ( self, data ) :
        chk = 0
        for i in range(0, len(data), 4):
            chk += up("<I", data[i:i+4])[0]
            chk &= 0xFFFF_FFFF
        return chk

    # Calculate payload checksum for uart loader
    # ( byte by byte )
    def chksum ( self, data ) :
        chk = 0
        for d in data:
            chk += d
            chk &= 0xFFFF_FFFF
        return chk

    # Add to payload checksum
    def add_chksum ( self, data ) :
        chk = self.sum
        for d in data:
            chk += d
            chk &= 0xFFFF_FFFF
        self.sum = chk

    # Original hack
    def gen_hdr ( self ):
        # xip ivt
        #hdr += p("<I", 0xeafffffe)
        hdr = p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        hdr += p("<I", 0xeafffffe)
        # width detect
        hdr += p("<I", 0xaa995566)
        hdr += b'XNLX'
        # encryption + misc
        hdr += p("<II", 0, 0x01010000)
        # :D ('source offset' - why yes, I'm like to boot the bootrom!)
        hdr += p("<I", 0x1_0000_0000-0x40000)
        # len
        hdr += p("<I", 0x2_0000)
        # load addr 0 or 0x4_0000 only...
        hdr += p("<I", 0)
        # entrypt (just a loop :))
        hdr += p("<I", 0x0FCB4)
        #"total image len" doesn't matter here
        hdr += p("<I", 0x010014)
        # QSPI something something
        hdr += p("<I", 1)
        # checksum
        hdr += p("<I", 0xffff_ffff - self.hdrchksum(hdr[0x20:]))

        # unused...
        for _ in range(19):
            hdr += p("<I", 0)
        # not sure at allll:
        hdr += p("<II", 0x8c0,0x8c0)
        # init lists
        for _ in range(0x100):
            hdr += p("<II", 0xffff_ffff, 0)
        return hdr

    # This is the original "trick" that 404 discovered
    # copied into this code framework.
    #
    # This should print:
    # checksum: 0x425b9
    # len: 2208
    def the_hack ( self ) :

        ser = self.ser

        img = self.gen_hdr()

        size = len(img)
        ##checksum = self.chksum(img)
        self.sum = 0
        self.add_chksum ( img )
        checksum = self.sum

        print("checksum: "+hex(checksum))
        print("len: "+str(size))

        print ( 'Waiting for XLNX-ZYNQ' )
        while ser.read(1) != b'X':
            continue
        assert ser.read(8) == b'LNX-ZYNQ'
        print ( "Got it" );

        n = ser.write(b"BAUD")

        #print ( "Write 1", n );
        baudgen = 0x11
        n = ser.write(baudgen.to_bytes(4, 'little'))
        #print ( "Write 2", n );
        reg0 = 0x6
        n = ser.write(reg0.to_bytes(4, 'little'))
        #print ( "Write 3", n );
        n = ser.write(size.to_bytes(4, 'little'))
        #print ( "Write 4", n );
        n = ser.write(checksum.to_bytes(4, 'little'))
        #print ( "Write 5", n );
        ser.flush ()

        print ( "Baud set, now write header image" )
        #print("writing image...")

        # sleep here 'cause this is where they hit resets for the tx/rx logic,
        # and anything in-flight when that happens is lost (it happens a fair bit)
        time.sleep(0.1)
        x = ser.write(img)
        ser.flush ()

        print ( "wrote header, byes: ", x )
        # let any error logic propagate..
        time.sleep(0.1)

        n = ser.in_waiting
        print ( n, " waiting" )

        time.sleep(0.5)
        n = ser.in_waiting
        print ( n, " waiting" )

        time.sleep(1.0)
        n = ser.in_waiting
        print ( n, " waiting" )

        if ser.in_waiting == 0:
            print("ok, i think we are done, ROM is 0x2_0000 bytes starting at 0 :)")
        else:
            print("something went wrong? bootrom says: " + str(ser.read(ser.in_waiting)))

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

        #src_offset = 0x900
        #src_pad = 6*4
        src_offset = 0x8c0
        src_pad = 2*4

        # 0x30 - source offset
        # :D ('source offset' - why yes, I'm like to boot the bootrom!)
        # hdr += p("<I", 0x1_0000_0000-0x40000)
        # Offset in bytes from the beginning of the bootrom header
        #  to where the FSBL (User code) starts.
        # Must be on a 64 byte boundary (0, 0x40, 0x80, 0xc0)
        #hdr += p("<I", im_offset)
        hdr += p("<I", src_offset)

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

        # 0x8a0 to 0x8fc
        # Pad so our image can follow
        # at 0x900
        for _ in range(src_pad):
            hdr += p("<I", 0xaaaa_aaaa)

        # a Note on this padding.
        # The code must start on a 64 byte boundary.
        # We pad with 96 bytes
        # We could pad with 32 bytes (to 0x8c0)
        # and change the offset above.

        return hdr

    def load ( self ) :
        ser = self.ser

        # This is "no change", i.e.
        # 115200 baud, no parity, 8 1
        uart1 = 0x11    # baud generator
        uart2 = 0x6
        b_uart1 = uart1.to_bytes(4, 'little')
        b_uart2 = uart2.to_bytes(4, 'little')

        # We need the bootrom header in order
        # to calculate the checksum.
        header = self.mk_header ( self.size )

        # Tell it to skip the checksum check
        # (just pass 0 as a checksum)
        # skip_checksum = True
        skip_checksum = False

        # Here we have an overall checksum that is
        # solely handled by the uart boot code.
        self.sum = 0
        if not skip_checksum :
            self.add_chksum ( header )
            if self.size > 0 :
                self.add_chksum ( self.image )

        b_cksum = self.sum.to_bytes(4, 'little')

        # size is header plus image
        size = len ( header ) + self.size
        b_size = size.to_bytes(4, 'little')

        ser.write ( b"BAUD" )
        ser.write ( b_uart1 )
        ser.write ( b_uart2 )
        ser.write ( b_size )

        ser.write ( b_cksum )

        time.sleep(0.01)

        print ( "Write header: ", len(header), " bytes" )
        ser.write ( header )

        if self.size > 0 :
            print ( "Write image: ", self.size, " bytes" )
            #print ( "Write image: ", len(self.image), " bytes" )
            ser.write ( self.image )

    def extra ( self, count ) :
        for _ in range(count) :
            self.ser.write(b"X")
            time.sleep(0.2)
            n = self.ser.in_waiting
            if n != 0 :
                break
        if n != 0 :
            self.show ( num, 999 )

    # This works nicely to show any errors coming
    # from the bootrom.
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

    # More persistent listening.
    def listen2 ( self ) :
        num = 0
        for _ in range(10) :
            num += 1
            time.sleep(0.1)
            n = self.ser.in_waiting
            if n != 0 :
                self.show ( num, n )
            else :
                print ( "-- Zip" )

# During debug, I got 0x2111 when I set the serial size to the header,
# but had non-zero image size.
# Then I sent a header with a zero image length, this gave
# me error 0x201e.
# As of 6/2/2024 we got the loader working.

original = False
zero = False

if original :
    test = Zynq ()
    test.the_hack ()
    test.extra ( 20 )
    test.listen ()
    print ( "Finished" )
    exit ()

if len(sys.argv) < 2 :
    print ( "Usage: loader xyz.bin" )
    exit ()

#binfile = "lots.bin"
binfile = sys.argv[1]

test = Zynq ()
if not zero :
    test.read_image ( binfile )
test.wait ()
test.load ()
#test.listen ()
test.listen2 ()

print ( "Done" )

# THE END
