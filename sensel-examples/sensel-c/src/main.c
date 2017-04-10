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
    
	senselGetDeviceList(&list);
	if (list.num_devices == 0)
	{
		fprintf(stdout, "No device found\n");
		return 0;
	}
	senselOpenDeviceByID(&handle, list.devices[0].idx);
	
	senselSetFrameContent(handle, FRAME_CONTENT_PRESSURE_MASK | FRAME_CONTENT_LABELS_MASK | FRAME_CONTENT_CONTACTS_MASK);
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
			fprintf(stdout, "Num Contacts: %d\n", frame->n_contacts);
            for (int i = 0; i < frame->n_contacts; i++)
            {
                fprintf(stdout, "Contact ID: %d\n", frame->contacts[i].id);
            }
            fprintf(stdout, "Forces:");
            for (int i = 0; i < 10; i++)
            {
                fprintf(stdout, " [%f] ", frame->force_array[i]);
            }
            fprintf(stdout, "\n");
            fprintf(stdout, "Labels:");
            for (int i = 0; i < 10; i++)
            {
                fprintf(stdout, " [%d] ", frame->labels_array[i]);
            }
            fprintf(stdout, "\n");
		}
	}
	return 0;
}
