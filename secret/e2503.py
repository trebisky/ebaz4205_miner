#!/bin/env python3

# provoke error 0x2502 from the secret bootrom
#  Tom Trebisky  5-19-2024

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


test = Zynq ()
test.wait ()
#test.e2501 ()
#test.e2502 ()
test.e2503 ()

# We see"
#  b'\r\n\r\nDec  1 2012 - 01:16:55\r\n3\r\nOutputStatus = 0x2502\r\xfe'

# THE END
