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

#ifndef __SENSEL_SERIAL_H__
#define __SENSEL_SERIAL_H__

#include "sensel_device.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned char senselSerialOpen                  (SenselSerialHandle *data, char* com_port);
unsigned char senselSerialOpenDeviceByID        (SenselSerialHandle *data, unsigned char idx);
unsigned char senselSerialOpenDeviceBySerialNum (SenselSerialHandle *data, char* serial_number);
unsigned char senselSerialOpenDeviceByComPort   (SenselSerialHandle *data, char* com_port);
unsigned char senselSerialScan                  (SenselDeviceList *list);
unsigned char senselSerialWrite                 (SenselSerialHandle *data, unsigned char* buf, int buf_len);
int           senselSerialReadAvailable         (SenselSerialHandle *data, unsigned char* buf, int buf_len);
unsigned char senselSerialReadBytes             (SenselSerialHandle *data, unsigned char* buf, int buf_len);
int           senselSerialGetAvailable          (SenselSerialHandle *data); // Checks number of available bytes
void          senselSerialFlushInput            (SenselSerialHandle *data);
void          senselSerialClose                 (SenselSerialHandle *data);

#ifdef __cplusplus
}
#endif

#endif //__SENSEL_SERIAL_H__
