#!/usr/bin/env python

##########################################################################
# Copyright 2013-2017 Sensel, Inc
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this
# software and associated documentation files (the "Software"), to deal in the Software
# without restriction, including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
# to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
##########################################################################

import sensel
import binascii

def openSensel():
    handle = None
    (error, device_list) = sensel.getDeviceList()
    if device_list.num_devices != 0:
        (error, handle) = sensel.openDeviceByID(device_list.devices[0].id)
    return handle

def initFrame():
    error = sensel.setFrameContent(handle, sensel.SENSEL_FRAME_CONTACTS_FLAG|sensel.SENSEL_FRAME_PRESSURE_FLAG)
    (error, frame) = sensel.allocateFrameData(handle)
    error = sensel.startScanning(handle)
    return frame

def scanFrames(frame):
    error = sensel.readSensor(handle)
    (error, num_frames) = sensel.getNumAvailableFrames(handle)
    for i in range(num_frames):
        error = sensel.getFrame(handle, frame)

def printFrame(frame):
    print "Num Contacts: ", frame.n_contacts
    for n in range(frame.n_contacts):
        c = frame.contacts[n]
        print "Contact ID: ", c.id
    forces = "Forces: "
    for n in range(10):
        forces += "["+str(frame.force_array[n])+"]"
    print forces

def closeSensel(frame):
    error = sensel.freeFrameData(handle, frame)
    error = sensel.stopScanning(handle)
    error = sensel.close(handle)

if __name__ == "__main__":
    handle = openSensel()
    if handle != None:
        frame = initFrame()
        for i in range(500):
            scanFrames(frame)
            printFrame(frame)
        closeSensel(frame)
    
