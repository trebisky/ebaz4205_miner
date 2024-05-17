#!/bin/env python3

# Here is the script to dump the bootrom
# using the secret uart loader.
# This also was written by "404"

from struct import pack as p
from struct import unpack as up
import serial
import time
import sys


baudgen = 0x11
reg0 = 0x6

def chksum(data):
    chk = 0
    for d in data:
        chk += d
        chk &= 0xFFFF_FFFF
    return chk

def hdrchksum(data):
    chk = 0
    for i in range(0, len(data), 4):
        chk += up("<I", data[i:i+4])[0]
        chk &= 0xFFFF_FFFF
    return chk

def gen_hdr():
    # xip ivt
    hdr += p("<I", 0xeafffffe)
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
    hdr += p("<I", 0xffff_ffff - hdrchksum(hdr[0x20:]))

    # unused...
    for _ in range(19):
        hdr += p("<I", 0)
    # not sure at allll:
    hdr += p("<II", 0x8c0,0x8c0)
    # init lists
    for _ in range(0x100):
        hdr += p("<II", 0xffff_ffff, 0)
    return hdr

img = gen_hdr()
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
if ser.in_waiting == 0:
    print("ok, i think we are done, ROM is 0x2_0000 bytes starting at 0 :)")
else:
    print("something went wrong? bootrom says: " + str(ser.read(ser.in_waiting)))

# THE END
