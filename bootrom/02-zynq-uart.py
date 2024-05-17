#!/bin/env python3

# This is the proof of concept script
# written by "404"

import serial
import time
import sys

if len(sys.argv) < 2:
    print("gimme a file")
    sys.exit(-1)

binfile = sys.argv[1]
img = open(binfile, 'rb').read()
baudgen = 0x11
reg0 = 0x6

def chksum(data):
    chk = 0
    for d in data:
        chk += d
    return chk

def dbgwrite(ser, data):
    print(str(data))
    ser.write(data)

size = len(img)
checksum = chksum(img)
print("checksum: "+hex(checksum))
print("len: "+str(size))


ser = serial.Serial(timeout=0.5)
ser.port = "/dev/ttyUSB0"
ser.baudrate = 115200
ser.open()

while ser.read(1) != b'X':
    continue
assert ser.read(8) == b'LNX-ZYNQ'

#size = 0xFFFFFFFE # :<
ser.write(b"BAUD")
ser.write(baudgen.to_bytes(4, 'little'))
ser.write(reg0.to_bytes(4, 'little'))
ser.write(size.to_bytes(4, 'little'))
ser.write(checksum.to_bytes(4, 'little'))

print("writing image...")
# sleep here 'cause this is where they hit resets for the tx/rx logic,
# and anything in-flight when that happens is lost (it happens a fair bit)
time.sleep(0.1)
print("wrote: " + str(ser.write(img)))
# let any error logic propagate..
time.sleep(0.1)
print("bootrom sez: " + str(ser.read(ser.in_waiting)))
