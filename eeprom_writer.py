#!/usr/bin/env python
# -*- coding: utf-8 -*-

import serial
import os

port = '/dev/ttyACM0'
ser = serial.Serial(port, baudrate=115200)

def PAGE_SIZE(): return 32
def ROM_SIZE(): return 32768

def write(address, fname):
    if os.path.isfile(fname):
        fsize = os.path.getsize(fname)
        with open(fname, 'r') as f:
            ser.write('w'.encode('utf-8'))
            ser.write(chr((address >> 8) & 0xFF))
            ser.write(chr(address & 0xFF))
            
            ser.write(chr((fsize >> 8) & 0xFF))
            ser.write(chr(fsize & 0xFF))
            b = f.read(1)
            print type(b)
            while b != '':
                #print b.encode('utf-8')
                #ser.write(chr(b))#.encode('utf-8'))
                ser.write(b)
                b = f.read(1)
                
            '''while (fsize > 0):
                data_size = PAGE_SIZE()
                page_offset = address % PAGE_SIZE()
                if (data_size > fsize):
                    data_size = fsize
                    
                ser.write(chr((data_size >> 8) & 0xFF))
                ser.write(chr(data_size & 0xFF))
                
                for i in range(data_size):
                    b = f.read(1)
                    ser.write(b.encode('utf-8'))
                
                processed = ord(ser.read(size=1))
                print processed
                fsize -= data_size
                
            ser.write(chr(0))'''
                
        result = ord(ser.read(size=1))
        return result
    else:
        print 'Not a file'
        return 0

def read(address):
    ser.write('r'.encode('utf-8'))
    ser.write(chr((address >> 8) & 0xFF))
    ser.write(chr(address & 0xFF))
    out = ord(ser.read(size=1))
    print out
    return out
    
def poke(address, data):
    ser.write('p'.encode('utf-8'))
    ser.write(chr((address >> 8) & 0xFF))
    ser.write(chr(address & 0xFF))
    ser.write(chr(data & 0xFF))
    result = ord(ser.read(size=1))
    return result
    
def dump(fname='dump'):
    ser.write('d'.encode('utf-8'))
    with open(fname, 'w') as f:
        f.write(ser.read(size=ROM_SIZE()))

def main(args):
    print ser.timeout
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
                    print '0x' + format(read(int(uinput[1], 0)), '02x')
            elif uinput[0] == 'poke' or uinput[0] == 'p':
                if len(uinput) != 3:
                    print 'Invalid number of arguments'
                else:
                    poke(int(uinput[1], 0), int(uinput[2], 0))
            elif uinput[0] == 'write' or uinput[0] == 'w':
                if (len(uinput) != 3):
                    print 'Invalid number of arguments'
                else:
                    print uinput[1]
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
                print '\tread [address]'
                print '\tpoke [address] [data]'
                print '\twrite [address] [file]'
                print '\tlock'
                print '\tunlock'
                print '\thelp'
                print '\tquit'
            else:
                print 'Invalid command'
    ser.close()
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
