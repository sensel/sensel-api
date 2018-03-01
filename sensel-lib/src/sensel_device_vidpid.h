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

#ifndef __SENSEL_DEVICE_VIDPID__
#define __SENSEL_DEVICE_VIDPID__

typedef struct device_vidpid_s
{
  char            *name;
  unsigned short  vid;
  char            *vidpidstr;
} DeviceVIDPID;

// List of all VID PIDs this API supports
DeviceVIDPID supported_devices[] =
{
  {  "Sensel Device" , 0x2C2F, "VID_2C2F" },
};

#define NUM_SUPPORTED_DEVS ((sizeof(supported_devices) / sizeof(DeviceVIDPID)))

#endif //__SENSEL_DEVICE_VIDPID__
