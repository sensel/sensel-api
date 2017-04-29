/******************************************************************************************
* MIT License
*
* Copyright (c) 2013-2017 Sensel, Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
******************************************************************************************/

#ifndef __SENSEL_REGISTER_H__
#define __SENSEL_REGISTER_H__

#include "sensel_device.h"

#define PT_READ_ACK           1
#define PT_READ_NACK          2
#define PT_RVS_ACK            3
#define PT_RVS_NACK           4
#define PT_WRITE_ACK          5
#define PT_WRITE_NACK         6
#define PT_WVS_ACK            7
#define PT_WVS_NACK           8
#define PT_ASYNC_DATA         9
#define PT_BUFFERED_FRAME     10

#define SENSEL_REG_DEPRECATED_SENSOR_COL_ACTIVE_COUNT  0x10
#define SENSEL_REG_DEPRECATED_SENSOR_ROW_ACTIVE_COUNT  0x11
#define SENSEL_REG_DEPRECATED_SENSOR_COL_INTERP_FACTOR 0x12
#define SENSEL_REG_DEPRECATED_SENSOR_ROW_INTERP_FACTOR 0x13

#define MAX_VS_PACKET_SIZE                             512
#define DEFAULT_VS_HEADER_SIZE                         4

#ifdef __cplusplus
extern "C" {
#endif

SenselStatus _senselReadReg(SENSEL_HANDLE handle, SenselSerialHandle *serial,
                            unsigned char reg, unsigned char size, unsigned char *buf);
SenselStatus _senselWriteReg(SENSEL_HANDLE handle, SenselSerialHandle *serial,
                             unsigned char reg, unsigned char size, unsigned char *buf);
SenselStatus _senselReadRegVS(SENSEL_HANDLE handle, SenselSerialHandle *serial,
                              unsigned char reg, unsigned int buf_size, unsigned char *buf, unsigned int *read_size);
SenselStatus _senselWriteRegVS(SENSEL_HANDLE handle, SenselSerialHandle *serial,
                               unsigned char reg, unsigned int size, unsigned char *buf, unsigned int *write_size);

#ifdef __cplusplus
}
#endif

#endif //__SENSEL_REGISTER_H__
