#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# /ADK/tools/usb_capture_analyzer.py
#
# This file is a part of ADK library.
# Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
# All rights reserved.
# See COPYING file for copyright details.

'''
Tool for analyzing captured USB traffic oscillograms. It accepts CSV file with
the data which can be optionally zip-compressed. Captures should contain data
for both D- and D+ lines.
'''

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

from optparse import OptionParser
import zipfile
import csv
import matplotlib
import numpy as np
import pylab
import math
import socket

def Error(msg, exception = None):
    print('ERROR: ' + msg);
    print('===================================================================')
    if exception is None:
        raise Exception(msg)
    else:
        raise

def Warning(msg, event = None):
    if event is None:
        print('WARNING: ' + msg)
    else:
        print('WARNING: %s %s' % (str(event), msg))

usage = '''
%prog [options] <file_name>
'''

# Global configuration variables and constants, taken from the specification
Vil = 0.8 # Input level low, V
Vih = 2.0 # Input level high, V
Vdi = 0.2 # Differential input sensitivity, V
Tldrate_min = 1.4775 # Low-speed data rate, min, Mb/s
Tldrate_max = 1.5225 # Low-speed data rate, max, Mb/s
Trf_min = 75e-9 # Rise-fall time, min, s
Trf_max = 300e-9 # Rise-fall time, min, s

Tperiod_min = 1.0 / (Tldrate_max * 1e6)
Tperiod_max = 1.0 / (Tldrate_min * 1e6)

################################################################################

def ImportData():
    '''
    Import data from the input file to the numpy arrays.
    '''
    global opts
    
    print('Processing file "%s"' % opts.filename)
    if opts.filename.endswith('.zip'):
        z = zipfile.ZipFile(opts.filename, 'r')
        f = z.open(z.infolist()[0], 'r')
    else:
        f = open(opts.filename, 'r')
    
    reader = csv.reader(f)
    # Skip several first rows with title
    timeData = list()
    dMinusData = list()
    dPlusData = list()
    skipRows = 2
    for row in reader:
        if skipRows > 0:
            skipRows -= 1
            continue
        timeData.append(float(row[0]))
        if opts.dMinus == 1:
            dMinusData.append(float(row[1]))
            dPlusData.append(float(row[2]))
        else:
            dMinusData.append(float(row[2]))
            dPlusData.append(float(row[1]))
    
    opts.timeData = np.array(timeData)
    opts.dMinusData = np.array(dMinusData)
    opts.dPlusData = np.array(dPlusData)
    
    print('%d samples imported' % len(timeData))

def HexDump(data):
    line = None
    for idx in range(0, 8 * int((len(data) + 7) / 8)):
        if (idx & 0x7) == 0:
            if line is not None:
                print(line)
            line = '%02x: ' % idx
        if (idx & 0x7) == 4:
            line += ' '
        if idx >= len(data):
            line += '.. '
        else:
            line += '%02x ' % data[idx]
        
        if (idx & 0x7) == 7:
            line += ' |'
            startIdx = idx & ~0x7
            endIdx = startIdx + 8
            if endIdx > len(data):
                endIdx = len(data)
            for byte in data[startIdx : endIdx]:
                if byte < 0x20 or byte > 127:
                    line += '.'
                else:
                    line += chr(byte)
            line += '|'
    
    if line is not None:
        print(line)

def Crc16(data):
    '''
    Calculation of USB CRC16 checksum for the specified data. This function
    uses exactly the same algorithm which is used in MCU in order to verify it
    and provide implementation reference.
    Used polynomial is CRC16-ANSI - X^16 + X^15+ X^2 + 1
    '''
    resid = 0xffff
    
    for byte in data:
        x = (byte ^ resid) & 0xff
        
        # Get parity of x
        parity = x ^ (x >> 4)
        parity = parity ^ (parity >> 2)
        parity = (parity ^ (parity >> 1)) & 0x1
        
        x = (x << 6) ^ (x << 7)
        if parity != 0:
            x = x ^ 0xc001
        
        resid = x ^ (resid >> 8)
    
    # Convert to network byte order
    return socket.htons((~resid) & 0xffff);

def PlotData():
    'Plot current capture data'
    global opts
    
    pylab.plot(opts.timeData, opts.dMinusData,
               opts.timeData, opts.dPlusData)
    pylab.grid()
    pylab.legend(('D-', 'D+'))
    pylab.show()

class Event(object):
    (TYPE_J,        # Line J state
     TYPE_K,        # Line K state
     TYPE_SE0,      # Line SE0 state
     TYPE_SE1,      # Line SE1 state
     TYPE_RESET,    # Reset signal
     TYPE_IDLE      # Idle state
     ) = range(0, 6)
    
    def __init__(self):
        self.startTime = None
        self.endTime = None
        self.type = None
    
    @staticmethod
    def GetTypeStr(type):
        if type == Event.TYPE_J:
            return 'J'
        if type == Event.TYPE_K:
            return 'K'
        if type == Event.TYPE_SE0:
            return 'SE0'
        if type == Event.TYPE_SE1:
            return 'SE1'
        if type == Event.TYPE_RESET:
            return 'RESET'
        if type == Event.TYPE_IDLE:
            return 'IDLE'
        return 'NONE'
    
    def __str__(self):
        return '[%g %.2fus %s]' % (self.startTime, 
                                   (self.endTime - self.startTime) * 1e6, 
                                   Event.GetTypeStr(self.type))
    
    def Duration(self):
        return self.endTime - self.startTime
    
    def _GetType(self, dminus, dplus):
        if dminus < Vil and dplus < Vil:
            return Event.TYPE_SE0
        elif dminus > Vih and dplus > Vih:
            return Event.TYPE_SE1
        elif dplus - dminus > Vdi:
            return Event.TYPE_K
        elif dminus - dplus > Vdi:
            return Event.TYPE_J
        return None
    
    def Feed(self, time, dminus, dplus):
        '''
        Feed the event with the samples.
        Return: True if the sample accepted, False if the sample is related to
        the next event.
        '''
        if self.startTime is None:
            self.startTime = time
            self.endTime = time
            self.type = self._GetType(dminus, dplus)
            return True
        self.endTime = time
        if self.type is None:
            self.type = self._GetType(dminus, dplus)
            return True
        type = self._GetType(dminus, dplus)
        if type == self.type:
            if self.type == Event.TYPE_J and self.endTime - self.startTime > 6.0 * Tperiod_max:
                # J state longer than 6 bits - bus is idle
                self.type = Event.TYPE_IDLE
            elif self.type == Event.TYPE_SE0 and self.endTime - self.startTime > 2.5e-6:
                # SE0 longer than 2.5us - bus reset
                self.type = Event.TYPE_RESET
            return True
        if self.type == Event.TYPE_IDLE and type == Event.TYPE_J:
            return True
        if self.type == Event.TYPE_RESET and type == Event.TYPE_SE0:
            return True
        # Next event detected
        return False
    
class Packet:
    
    def __init__(self, isKeepAlive = False):
        self.isKeepAlive = isKeepAlive
        self.events = list()
        self.syncPatternPos = 0
        self.isInvalid = False
        # Last line state for NRZI, 0 - J, 1 - K
        self.lastState = 1
        self.stuffCount = 5
        self.bytes = list()
        self.bitCount = 0
        self.curByte = 0
        self.crcOk = None
        self.crc = None
        self.crcPkt = None
        
    def __len__(self):
        return len(self.bytes)
        
    def _AddBit(self, bit, event):
        'Add bit, either zero or one'
        if self.isInvalid:
            return
        
        if bit == 1:
            if self.stuffCount == 0:
                Warning('Stuff error', event)
                self.isInvalid = True
                return
            self.stuffCount -= 1
            self.curByte = (self.curByte >> 1) | 0x80
        else:
            if self.stuffCount == 0:
                # Skip stuffing bit
                self.stuffCount = 6
                return
            self.stuffCount = 6
            self.curByte = self.curByte >> 1
        
        self.bitCount += 1
        if self.bitCount == 8:
            self.bitCount = 0
            self.bytes.append(self.curByte)
            self.curByte = 0
        
    def _AddBits(self, event, numBits):
        if event.type != Event.TYPE_J and event.type != Event.TYPE_K:
            Error('Wrong event type supplied')
        state = 1 if event.type == Event.TYPE_K else 0
        if state == self.lastState:
            for i in range(0, numBits):
                self._AddBit(1, event)
        else:
            self.lastState = state
            self._AddBit(0, event)
            for i in range(1, numBits):
                self._AddBit(1, event)
        
    def _GetNumBits(self, event):
        duration = event.Duration()
        num = int(duration / ((Tperiod_min + Tperiod_max) / 2.0) + 0.5)
        if duration < Tperiod_min * num:
            Warning('Signal too short for %d bits train, %g < %g' % (num, duration, Tperiod_min * num), event)
        elif duration > Tperiod_max * num + Trf_max:
            Warning('Signal too long for %d bits train, %g > %g' % (num, duration, Tperiod_max * num), event)
        return num
        
    def _ProcessSync(self, event):
        if self.syncPatternPos < 6:
            expectedType = Event.TYPE_K if self.syncPatternPos & 1 == 0 else Event.TYPE_J
        else:
            expectedType = Event.TYPE_K
        
        if event.type != expectedType:
            Warning('Wrong sync pattern, expected %s, have %s' % 
                    (Event.GetTypeStr(expectedType), Event.GetTypeStr(event.type)),
                    event)
            self.isInvalid = True
            return
        
        n = self._GetNumBits(event)
        if self.syncPatternPos < 6:
            if n != 1:
                Warning('Expected one bit, have %d' % n, event)
                self.isInvalid = True
                return
        else:
            if n < 2:
                Warning('Expected at least two bits', event)
                self.isInvalid = True
                return
            if n > 2:
                self._AddBits(event, n - 2)
        
        self.syncPatternPos += 1
    
    def AddEvent(self, event):
        self.events.append(event)
        if self.isKeepAlive:
            return
        
        if self.isInvalid:
            # Do not try to process if already out of sync
            return
        
        if self.syncPatternPos < 7:
            self._ProcessSync(event)
            return
        
        if event.type == Event.TYPE_RESET:
            Warning('Reset in the middle of packet', event)
            self.isInvalid = True
            self.Finalize()
            return
        
        if event.type == Event.TYPE_SE0:
            if event.Duration() < Tperiod_min:
                Warning('Too short EOP', event)
            elif event.Duration() > Tperiod_max * 3:
                Warning('Too long EOP', event)
            self.Finalize()
            return
        
        n = self._GetNumBits(event)
        self._AddBits(event, n)
    
    def Finalize(self):
        if self.bitCount != 0:
            Warning('Packet byte truncated', self.events[-1])
            self.isInvalid = True
        if not self.isInvalid and not self.isKeepAlive:
            self.pid = self.bytes[0] & 0xf
            if self.pid ^ (self.bytes[0] >> 4) != 0xf:
                Warning('PID verification failed, 0x%x' % (self.bytes[0]), self.events[0])
                self.isInvalid = True
            else:
                # Check CRC for data packets
                pid = self.GetPidStr(self.pid)
                if pid == 'DATA0' or pid == 'DATA1' or pid == 'DATA2' or pid == 'MDATA':
                    self.crc = Crc16(self.bytes[1 : -2])
                    self.crcPkt = (self.bytes[-2] << 8) | self.bytes[-1]
                    self.crcOk = self.crcPkt == self.crc
                    if not self.crcOk:
                        Warning('Wrong CRC, have %04x, should be %04x' % (self.crcPkt, self.crc), self.events[0])
                        self.isInvalid = True
    
    def GetPidStr(self, pid):
        if pid == 0b0001:
            return 'OUT'
        if pid == 0b1001:
            return 'IN'
        if pid == 0b0101:
            return 'SOF'
        if pid == 0b1101:
            return 'SETUP'
        if pid == 0b0011:
            return 'DATA0'
        if pid == 0b1011:
            return 'DATA1'
        if pid == 0b0111:
            return 'DATA2'
        if pid == 0b1111:
            return 'MDATA'
        if pid == 0b0010:
            return 'ACK'
        if pid == 0b1010:
            return 'NACK'
        if pid == 0b1110:
            return 'STALL'
        if pid == 0b0110:
            return 'NYET'
        if pid == 0b1100:
            return 'PRE'
        if pid == 0b1000:
            return 'SPLIT'
        if pid == 0b0100:
            return 'PING'
    
    def Dump(self):
        global opts
        
        if self.isKeepAlive:
            pid = 'KEEP-ALIVE'
        else:
            pid = self.GetPidStr(self.bytes[0] & 0xf)
        
        if self.crcOk is not None:
            if self.crcOk:
                crcStr = ' CRC OK'
            else:
                crcStr = ' CRC ERR %04x/%04x' % (self.crcPkt, self.crc)
        else:
            crcStr = ''
        
        print('\n[%s%g %s %d bytes%s]' % ('INVALID ' if self.isInvalid else '',
                                      self.events[0].startTime,
                                      pid,
                                      len(self.bytes),
                                      crcStr))
        HexDump(self.bytes)
        
        if opts.dbgLeds:
            def GetLed(x):
                s = ''
                for i in range(0, 4):
                    if (x & (1 << i)) == 0:
                        s += '.'
                    else:
                        s += '*'
                return s
            
            print('Debug LEDs sequence:')
            idx = 0
            for byte in self.bytes:
                print('%3d.  %s' % (idx, GetLed(byte & 0xf)))
                print('%3d.  %s' % (idx + 1, GetLed(byte >> 4)))
                idx += 2

def DigitizeData():
    'Extract bits and packets from oscillogram data.'
    
    global opts
    
    # Firstly create set of events found in the signal.
    opts.events = list()
    
    curEvent = None
    for idx in range(0, len(opts.timeData)):
        if curEvent is None:
            curEvent = Event()
        if not curEvent.Feed(opts.timeData[idx], opts.dMinusData[idx], opts.dPlusData[idx]):
            opts.events.append(curEvent)
            curEvent = Event()
            curEvent.Feed(opts.timeData[idx], opts.dMinusData[idx], opts.dPlusData[idx])
    if curEvent.type is not None:
        opts.events.append(curEvent)
    print('%d events recognized in the signal' % len(opts.events))
    
    opts.packets = list()
    curPacket = None
    # Analyze the events
    for event in opts.events:
        if curPacket is None:
            if event.type == Event.TYPE_J or event.type == Event.TYPE_IDLE:
                continue
            if event.type == Event.TYPE_SE0:
                # Should be keep-alive message
                if event.Duration() < Tperiod_min:
                    Warning('Too short keep-alive', event)
                elif event.Duration() > Tperiod_max * 3:
                    Warning('Too long keep-alive', event)
                    
                curPacket = Packet(True)
                curPacket.AddEvent(event)
                opts.events.append(curPacket)
                curPacket = None
                continue
            
            if event.type == Event.TYPE_K:
                curPacket = Packet()
                curPacket.AddEvent(event)
                continue
            
            continue
        
        curPacket.AddEvent(event)
        if event.type == Event.TYPE_SE0:
            # Consider it is EOP
            opts.packets.append(curPacket)
            curPacket = None
        
    if curPacket is not None:
        curPacket.Finalize()
        opts.packets.append(curPacket)
    
    opts.numInvalidPackets = 0
    for pkt in opts.packets:
        if pkt.isInvalid:
            opts.numInvalidPackets += 1
    print('%d packets found' % len(opts.packets))
    if opts.numInvalidPackets > 0:
        Warning('%d invalid packets' % opts.numInvalidPackets)
    
def DumpPackets():
    global opts
    
    for pkt in opts.packets:
        pkt.Dump()

################################################################################

def Main():
    global opts

    optParser = OptionParser(usage = usage)
    
    optParser.add_option('--dminus', dest = 'dMinus',
                         help = 'Specify which channel (1 or 2) contains capture ' +
                                'data for D- line.',
                         default = 1)
    optParser.add_option('--plot', dest = 'doPlot',
                         action = 'store_true',
                         help = 'Plot the data.',
                         default = False)
    optParser.add_option('--dbg-leds', dest = 'dbgLeds',
                         action = 'store_true',
                         help = 'Produce output for debug LEDs.',
                         default = False)

    (opts, args) = optParser.parse_args()

    if len(args) < 1:
        Error('File name should be provided as argument')
    opts.filename = args[0]
    
    if opts.dMinus != 1 and opts.dMinus != 2:
        Error('D- line should be bound to either channel 1 or 2')
    
    ImportData()
    DigitizeData()
    DumpPackets()
    
    if opts.doPlot:
        PlotData()

if __name__ == "__main__":
    Main()