#!/bin/env python3

# provoke error 0x2501 from the secret bootrom
#  Tom Trebisky  5-19-2024

# Error 0x2501 is when the secret loader does not see
# the character "B" after 128 characters have been read.

# On my Fedora 40 system, I needed:
# dnf install python3-pyserial

from struct import pack as p
from struct import unpack as up
import serial
import time
import sys

# =================================================

ser = serial.Serial(timeout=0.5)
ser.port = "/dev/ttyUSB0"
ser.baudrate = 115200
ser.open()

print ( 'Waiting for XLNX-ZYNQ' )
while ser.read(1) != b'X':
    continue
assert ser.read(8) == b'LNX-ZYNQ'
print ( "Got the XLNX-ZYNQ" );

def show ( ser, num, n ) :
    print ( "Waiting:", num, n );
    msg = ser.read (n )
    print ( msg )
    #print ( str(msg, encoding='utf-8') )
    #print ( str(msg, encoding="ISO-8859-1" ) )

num = 0
while True:
    time.sleep(0.01)
    num += 1
    ser.write(b"X")
    n = ser.in_waiting
    if n != 0 :
        show ( ser, num, n )
    if num > 500 :
        break

# I expected to see the error over and over (every 128 chars),
# but we only get one message:

# We see this:
# Waiting: 130 54
# b'\r\n\r\nDec  1 2012 - 01:16:55\r\n3\r\nOutputStatus = 0x2501\r\xfe'

# So we get 2 blank lines, a timestamp, the number "3",
# the error number (OutputStatus), and a nasty 0xfe byte.

# THE END
