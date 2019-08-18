#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
This program written by Gregory Norton <Gregory.Norton@me.com>

This program is used to interface with an
arduino running the included firmware. This
program is intended for use with 29C256 EEPROM,
but can likely be adapted for use with other chips.
'''

import serial
from enum import Enum
import os

class Protocol(Enum):
    FAIL = 0
    END = 1
    READ = 2
    WRITE = 3
    DUMP = 4
    
class ProtocolException(Exception):
    def __init__(self, msg = None, val = None):
        self.msg = msg
        self.val = val

port = '/dev/ttyACM0'
ser = serial.Serial(port, baudrate=115200)

def PAGE_SIZE(): return 64
def ROM_SIZE(): return 32768

def page(address): return (address >> 6) & 0x3FF
def page_offset(address): return address & 0x3F

def write(address, fname):
    if not os.path.isfile(fname):
        print 'File does not exist'
        return Protocol.FAIL
        
    fsize = os.path.getsize(fname)
    
    bufs = {}
    for pg in range(page(address), page(address + fsize) + 1):
        bufs[pg] = [0xFF] * PAGE_SIZE()
    
    with open(fname, 'r') as f:
        for current_address in range(address, address + fsize):
            bufs[page(current_address)][page_offset(current_address)] = ord(f.read(1))
    
    for i in bufs:
        page_address = i << 6
        ser.write(chr(Protocol.WRITE.value))
        ser.write(chr(page_address >> 8))
        ser.write(chr(page_address & 0xFF))
        for b in bufs[i]:
            ser.write(chr(b))
            
        result = ord(ser.read(size=1))
        if (result != Protocol.END.value):
            print hex(result)
            raise ProtocolException('Unexpected protocol message while awaiting response', end)
    return Protocol.END

def read(address):
    print address
    
    ser.write(chr(Protocol.READ.value))
    ser.write(chr((address >> 8) & 0xFF))
    ser.write(chr(address & 0xFF))
    
    out = ord(ser.read(size = 1))
    result = ord(ser.read(size = 1))
    if (result != Protocol.END.value):
        print hex(result)
        raise ProtocolException('Unexpected protocol message while awaiting response.', end)
    return out
    
def dump(fname='dump.bin'):
    ser.write(chr(Protocol.DUMP.value))
    with open(fname, 'w') as f:
        f.write(ser.read(size=ROM_SIZE()))
    end = ord(ser.read(size=1))
    if (end != Protocol.END.value):
        print hex(end)
        raise ProtocolException('Unexpected protocol message while awaiting response.', end)
        
    return end

def main(args):
    print ser.read_until()
    print 'Type help for information on available commands'
    print ''
    
    #ser.write(chr(Protocol.ADDRESS.value))
    #print ser.read_until()
    
    active = True
    while active:
        uinput = raw_input('> ')
        if uinput != '':
            uinput = uinput.split()
            if uinput[0] == 'quit' or uinput[0] == 'q' or uinput[0] == 'exit':
                active = False
            elif uinput[0] == 'read' or uinput[0] == 'r':
                if len(uinput) != 2:
                    print 'Invalid number of arguments'
                else:
                    print hex(read(int(uinput[1], 0)))
            elif uinput[0] == 'write' or uinput[0] == 'w':
                if (len(uinput) != 3):
                    print 'Invalid number of arguments'
                else:
                    write(int(uinput[1], 0), uinput[2])
            elif uinput[0] == 'lock' or uinput[0] == 'l':
                ser.write('l'.encode('utf-8'))
                result = ord(ser.read(size=1))
                print result
            elif uinput[0] == 'unlock' or uinput[0] == 'u':
                ser.write('u'.encode('utf-8'))
                result = ord(ser.read(size=1))
                print result
            elif uinput[0] == 'dump' or uinput[0] == 'd':
                dump()
            elif uinput[0] == 'help' or uinput[0] == 'h':
                print 'Available commands'
                print '\tread address1'# [address2]'
                #print '\tpoke address [data]'
                print '\twrite address file'
                #print '\tlock'
                #print '\tunlock'
                print '\thelp'
                print '\tquit'
            else:
                print 'Invalid command'
    ser.close()
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
