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

import sys

from ctypes import *
from sensel_register_map import *
import platform

sensel_lib = None
sensel_lib_decompress = None

SENSEL_MAX_DEVICES  = 16

FRAME_CONTENT_PRESSURE_MASK = 0x01
FRAME_CONTENT_LABELS_MASK   = 0x02
FRAME_CONTENT_CONTACTS_MASK = 0x04
FRAME_CONTENT_ACCEL_MASK    = 0x08

CONTACT_MASK_ELLIPSE        =   0x01
CONTACT_MASK_DELTAS         =   0x02
CONTACT_MASK_BOUNDING_BOX   =   0x04
CONTACT_MASK_PEAK           =   0x08

CONTACT_INVALID = 0
CONTACT_START   = 1
CONTACT_MOVE    = 2
CONTACT_END     = 3

platform_name = platform.system()
if platform_name == "Windows":
    python_is_x64 = sys.maxsize > 2**32
    if python_is_x64:
        sensel_lib_decompress = windll.LoadLibrary("C:\\Program Files\\Sensel\\SenselLib\\x64\\LibSenselDecompress.dll")
        sensel_lib = windll.LoadLibrary("C:\\Program Files\\Sensel\\SenselLib\\x64\\LibSensel.dll")
    else:
        sensel_lib_decompress = windll.LoadLibrary("C:\\Program Files\\Sensel\\SenselLib\\x86\\LibSenselDecompress.dll")
        sensel_lib = windll.LoadLibrary("C:\\Program Files\\Sensel\\SenselLib\\x86\\LibSensel.dll")
elif platform_name == "Darwin":
    sensel_lib = cdll.LoadLibrary("/usr/local/lib/libSensel.dylib")
else:
    sensel_lib = cdll.LoadLibrary("/usr/lib/libsensel.so")

class SenselSensorInfo(Structure):
    _fields_ = [("max_contacts", c_ubyte), 
                ("num_rows", c_ushort), 
                ("num_cols", c_ushort), 
                ("width", c_float), 
                ("height", c_float)] 

class SenselContact(Structure):
    _fields_ = [("content_bit_mask", c_ubyte), 
                ("id", c_ubyte), 
                ("state", c_int), 
                ("x_pos", c_float), 
                ("y_pos", c_float), 
                ("total_force", c_float), 
                ("area", c_float),
                ("orientation", c_float), 
                ("major_axis", c_float), 
                ("minor_axis", c_float), 
                ("delta_x", c_float),
                ("delta_y", c_float), 
                ("delta_force", c_float), 
                ("delta_area", c_float), 
                ("min_x", c_float),
                ("min_y", c_float), 
                ("max_x", c_float), 
                ("max_y", c_float),
                ("peak_x", c_float), 
                ("peak_y", c_float), 
                ("peak_force", c_float)] 

class SenselAccelData(Structure):
    _fields_ = [("x", c_int), 
                ("y", c_int), 
                ("z", c_int)] 

class SenselFrameData(Structure):
    _fields_ = [("content_bit_mask", c_ubyte),
                ("lost_frame_count", c_int), 
                ("n_contacts", c_ubyte), 
                ("contacts", POINTER(SenselContact)),
                ("force_array", POINTER(c_float)),
                ("labels_array", POINTER(c_ubyte)),
                ("accel_data", POINTER(SenselAccelData))]

class SenselDeviceID(Structure):
    _fields_ = [("idx", c_ubyte), 
                ("serial_num", c_ubyte*64), 
                ("com_port", c_ubyte*64)] 

class SenselDeviceList(Structure):
    _fields_ = [("num_devices", c_ubyte), 
                ("devices", SenselDeviceID*SENSEL_MAX_DEVICES)] 

def open():
    handle = c_void_p(0)
    error = sensel_lib.senselOpen(POINTER(handle))
    return (error, handle)

def getDeviceList():
    device_list = SenselDeviceList(0)
    for i in range(SENSEL_MAX_DEVICES):
        device_list.devices[i] = SenselDeviceID(0)
    error = sensel_lib.senselGetDeviceList(byref(device_list))
    return (error, device_list)

def openDeviceByID(idx):
    c_idx = c_ubyte(idx)
    handle = c_void_p(0)
    error = sensel_lib.senselOpenDeviceByID(byref(handle), c_idx)
    return (error, handle)

def close(handle):
    error = sensel_lib.senselClose(handle)
    return error
    
def softReset(handle):
    error = sensel_lib.senselSoftReset(handle)
    return error

def getSensorInfo(handle):
    info = SenselSensorInfo(0,0,0,0,0)
    error = sensel_lib.senselGetSensorInfo(handle, byref(info))
    return (error, info)

def allocateFrameData(handle):
    frame_pointer = POINTER(SenselFrameData)()
    error = sensel_lib.senselAllocateFrameData(handle, byref(frame_pointer))
    return (error, frame_pointer.contents)

def freeFrameData(handle, frame):
    error = sensel_lib.senselFreeFrameData(handle, byref(frame))
    return error

def setScanDetail(handle, detail):
    c_detail = c_int(detail)
    error = sensel_lib.senselSetScanDetail(handle, c_detail)
    return error

def getScanDetail(handle):
    detail = c_int(0)
    error = sensel_lib.senselGetScanDetail(handle, byref(detail))
    return (error, detail.value)

def getSupportedFrameContent(handle):
    content = c_ubyte(0)
    error = sensel_lib.senselGetSupportedFrameContent(handle, byref(content))
    return (error, content.value)

def setFrameContent(handle, content):
    c_content = c_ubyte(content)
    error = sensel_lib.senselSetFrameContent(handle, c_content)
    return error

def getFrameContent(handle):
    content = c_ubyte(0)
    error = sensel_lib.senselGetFrameContent(handle, byref(content))
    return (error, content.value)

def startScanning(handle):
    error = sensel_lib.senselStartScanning(handle)
    return error

def stopScanning(handle):
    error = sensel_lib.senselStopScanning(handle)
    return error

def readSensor(handle):
    error = sensel_lib.senselReadSensor(handle)
    return error

def getNumAvailableFrames(handle):
    num_frames = c_int(0)
    error = sensel_lib.senselGetNumAvailableFrames(handle, byref(num_frames))
    return (error, num_frames.value)

def getFrame(handle, frame):
    error = sensel_lib.senselGetFrame(handle, byref(frame))
    return error

def setLEDBrightness(handle, led_id, brightness):
    c_led_id = c_ubyte(led_id)
    c_brightness = c_ushort(brightness)
    error = sensel_lib.senselSetLEDBrightness(handle, c_led_id, c_brightness)
    return error;

def setContactsMask(handle, mask):
    c_mask = c_ubyte(mask)
    error = sensel_lib.senselSetContactsMask(handle, c_mask)
    return error

def getFrameContent(handle):
    mask = c_ubyte(0)
    error = sensel_lib.senselGetContactsMask(handle, byref(mask))
    return (error, content.value)

def readReg(handle, reg, size):
    buf = (c_byte * size)()
    error = sensel_lib.senselReadReg(handle, c_ubyte(reg), c_ubyte(size), buf)
    return (error, buf)

def writeReg(handle, reg, size, data):
    buf = (c_ubyte * size)(*data)
    error = sensel_lib.senselWriteReg(handle, c_ubyte(reg), c_ubyte(size), buf)
    return error

def readRegVS(handle, reg, size):
    buf = (c_byte * size)()
    read_size = c_int(0)
    error = sensel_lib.senselReadRegVS(handle, c_ubyte(reg), c_ubyte(size), buf, byref(read_size))
    return (error, buf, read_size)

def writeRegVS(handle, reg, size, data):
    buf = (c_byte * size)(*data)
    write_size = c_int(0)
    error = sensel_lib.senselReadRegVS(handle, c_ubyte(reg), c_ubyte(size), buf, byref(write_size))
    return (error, write_size)
