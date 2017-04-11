/******************************************************************************************
* Copyright 2013-2017 Sensel, Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this 
* software and associated documentation files (the "Software"), to deal in the Software 
* without restriction, including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
* to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
* PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
* FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
* OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
* DEALINGS IN THE SOFTWARE.
******************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensel.h"
#include "sensel_device.h"

#define TEST_SCAN_NUM_LOOPS 500

int main(int argc, char **argv)
{
    SenselDeviceList list;
    SENSEL_HANDLE handle = NULL;
    SenselFrameData *frame = NULL;
	SenselSensorInfo sensor_info;
	float total_force = 0.0f;

	senselGetDeviceList(&list);
	if (list.num_devices == 0)
	{
		fprintf(stdout, "No device found\n");
		return 0;
	}
	senselOpenDeviceByID(&handle, list.devices[0].idx);
	senselGetSensorInfo(handle, &sensor_info);
	
	senselSetFrameContent(handle, FRAME_CONTENT_PRESSURE_MASK);
	senselAllocateFrameData(handle, &frame);
	senselStartScanning(handle);
	for (int c = 0; c < TEST_SCAN_NUM_LOOPS; c++)
	{
		unsigned int num_frames = 0;
		senselReadSensor(handle);
		senselGetNumAvailableFrames(handle, &num_frames);
		for (int f = 0; f < num_frames; f++)
		{
			senselGetFrame(handle, frame);
			total_force = 0;
            for (int i = 0; i < sensor_info.num_cols*sensor_info.num_rows; i++)
            {
				total_force = total_force + frame->force_array[i];
            }
            fprintf(stdout, "Total Force : %f\n", total_force);
		}
	}
	return 0;
}
