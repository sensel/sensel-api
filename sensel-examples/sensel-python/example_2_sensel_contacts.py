#!/usr/bin/env python

##########################################################################
# MIT License
#
# Copyright (c) 2013-2017 Sensel, Inc.
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

# Python 3 compatibility
from __future__ import print_function
try:
    input = raw_input
except NameError:
    pass

import sys
import threading

sys.path.append('../../sensel-lib-wrappers/sensel-lib-python')
import sensel

enter_pressed = False


def wait_for_enter():
    global enter_pressed
    input('Press Enter to exit...')
    enter_pressed = True
    return


def open_sensel():
    handle = None
    (error, device_list) = sensel.getDeviceList()
    if device_list.num_devices != 0:
        (error, handle) = sensel.openDeviceByID(device_list.devices[0].idx)
    return handle


def init_frame():
    error = sensel.setFrameContent(handle, sensel.FRAME_CONTENT_CONTACTS_MASK)
    error, frame = sensel.allocateFrameData(handle)
    error = sensel.startScanning(handle)
    return frame


def scan_frames(frame, info):
    error = sensel.readSensor(handle)
    error, num_frames = sensel.getNumAvailableFrames(handle)
    for i in range(num_frames):
        error = sensel.getFrame(handle, frame)
        print_frame(frame, info)


def print_frame(frame, info):
    if frame.n_contacts:
        print()
        print('Num Contacts:', frame.n_contacts)
        for n in range(frame.n_contacts):
            c = frame.contacts[n]
            print('Contact ID:', c.id)
            if c.state == sensel.CONTACT_START:
                sensel.setLEDBrightness(handle, c.id, 100)
            elif c.state == sensel.CONTACT_END:
                sensel.setLEDBrightness(handle, c.id, 0)


def close_sensel(frame):
    error = sensel.freeFrameData(handle, frame)
    error = sensel.stopScanning(handle)
    error = sensel.close(handle)


if __name__ == '__main__':
    handle = open_sensel()
    if handle:
        error, info = sensel.getSensorInfo(handle)
        frame = init_frame()

        t = threading.Thread(target=wait_for_enter)
        t.start()
        while not enter_pressed:
            scan_frames(frame, info)
        close_sensel(frame)
