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

import sys
sys.path.append('../../sensel-lib-wrappers/sensel-lib-python')
import sensel

if __name__ == '__main__':
    handle = None
    error, device_list = sensel.getDeviceList()
    if device_list.num_devices:
        error, handle = sensel.openDeviceByID(device_list.devices[0].idx)
    if handle:
        error, info = sensel.getSensorInfo(handle)

        print('\nSensel Device: %s' % bytearray(device_list.devices[0].serial_num).decode('utf-8'))
        print('Width: %smm' % info.width)
        print('Height: %smm' % info.height)
        print('Cols: %s' % info.num_cols)
        print('Rows: %s' % info.num_rows)
        error = sensel.close(handle)
