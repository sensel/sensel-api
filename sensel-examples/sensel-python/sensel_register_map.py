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

SENSEL_REG_MAP_PROTOCOL_VERSION 	= 1
SENSEL_REG_MAP_MAJOR_VERSION 		= 0
SENSEL_REG_MAP_MINOR_VERSION 		= 7
SENSEL_REG_MAP_BUILD_VERSION 		= 60

SENSEL_REG_MAGIC                                    = 0x00
SENSEL_REG_FW_VERSION_PROTOCOL                      = 0x06
SENSEL_REG_FW_VERSION_MAJOR                         = 0x07
SENSEL_REG_FW_VERSION_MINOR                         = 0x08
SENSEL_REG_FW_VERSION_BUILD                         = 0x09
SENSEL_REG_FW_VERSION_RELEASE                       = 0x0B
SENSEL_REG_DEVICE_ID                                = 0x0C
SENSEL_REG_DEVICE_REVISION                          = 0x0E
SENSEL_REG_DEVICE_SERIAL_NUMBER                     = 0x0F
SENSEL_REG_SENSOR_NUM_COLS                          = 0x10
SENSEL_REG_SENSOR_NUM_ROWS                          = 0x12
SENSEL_REG_SENSOR_ACTIVE_AREA_WIDTH_UM              = 0x14
SENSEL_REG_SENSOR_ACTIVE_AREA_HEIGHT_UM             = 0x18
SENSEL_REG_SCAN_FRAME_RATE                          = 0x20
SENSEL_REG_SCAN_BUFFER_CONTROL                      = 0x22
SENSEL_REG_SCAN_DETAIL_CONTROL                      = 0x23
SENSEL_REG_FRAME_CONTENT_CONTROL                    = 0x24
SENSEL_REG_SCAN_ENABLED                             = 0x25
SENSEL_REG_SCAN_READ_FRAME                          = 0x26
SENSEL_REG_FRAME_CONTENT_SUPPORTED                  = 0x28
SENSEL_REG_CONTACTS_MAX_COUNT                       = 0x40
SENSEL_REG_CONTACTS_ENABLE_BLOB_MERGE               = 0x41
SENSEL_REG_CONTACTS_MIN_FORCE                       = 0x47
SENSEL_REG_CONTACTS_MASK                            = 0x4B
SENSEL_REG_BASELINE_ENABLED                         = 0x50
SENSEL_REG_BASELINE_INCREASE_RATE                   = 0x51
SENSEL_REG_BASELINE_DECREASE_RATE                   = 0x53
SENSEL_REG_BASELINE_DYNAMIC_ENABLED                 = 0x57
SENSEL_REG_ACCEL_X                                  = 0x60
SENSEL_REG_ACCEL_Y                                  = 0x62
SENSEL_REG_ACCEL_Z                                  = 0x64
SENSEL_REG_BATTERY_STATUS                           = 0x70
SENSEL_REG_BATTERY_PERCENTAGE                       = 0x71
SENSEL_REG_POWER_BUTTON_PRESSED                     = 0x72
SENSEL_REG_LED_BRIGHTNESS                           = 0x80
SENSEL_REG_LED_BRIGHTNESS_SIZE                      = 0x81
SENSEL_REG_LED_BRIGHTNESS_MAX                       = 0x82
SENSEL_REG_LED_COUNT                                = 0x84
SENSEL_REG_UNIT_SHIFT_DIMS                          = 0xA0
SENSEL_REG_UNIT_SHIFT_FORCE                         = 0xA1
SENSEL_REG_UNIT_SHIFT_AREA                          = 0xA2
SENSEL_REG_UNIT_SHIFT_ANGLE                         = 0xA3
SENSEL_REG_UNIT_SHIFT_TIME                          = 0xA4
SENSEL_REG_DEVICE_OPEN                              = 0xD0
SENSEL_REG_SOFT_RESET                               = 0xE0
SENSEL_REG_ERROR_CODE                               = 0xEC
