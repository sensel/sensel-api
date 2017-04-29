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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensel.h"
#include "sensel_device.h"

//The number of times the example should read from the Sensel device
#define TEST_SCAN_NUM_LOOPS 500

int main(int argc, char **argv)
{
	//Handle that references a Sensel device
	SENSEL_HANDLE handle = NULL;
	//List of all available Sensel devices
	SenselDeviceList list;
	//SenselFrame data that will hold the contacts
	SenselFrameData *frame = NULL;

	//Get a list of avaialble Sensel devices
	senselGetDeviceList(&list);
	if (list.num_devices == 0)
	{
		fprintf(stdout, "No device found\n");
		return 0;
	}

	//Open a Sensel device by the id in the SenselDeviceList, handle initialized 
	senselOpenDeviceByID(&handle, list.devices[0].idx);

	//Set the frame content to scan contact data
	senselSetFrameContent(handle, FRAME_CONTENT_CONTACTS_MASK);
	//Allocate a frame of data, must be done before reading frame data
	senselAllocateFrameData(handle, &frame);
	//Start scanning the Sensel device
	senselStartScanning(handle);
	for (int c = 0; c < TEST_SCAN_NUM_LOOPS; c++)
	{
		unsigned int num_frames = 0;
		//Read all available data from the Sensel device
		senselReadSensor(handle);
		//Get number of frames available in the data read from the sensor
		senselGetNumAvailableFrames(handle, &num_frames);
		for (int f = 0; f < num_frames; f++)
		{
			//Read one frame of data
			senselGetFrame(handle, frame);
			//Print out contact data
			fprintf(stdout, "Num Contacts: %d\n", frame->n_contacts);
            for (int i = 0; i < frame->n_contacts; i++)
            {
                fprintf(stdout, "Contact ID: %d\n", frame->contacts[i].id);
            }
            fprintf(stdout, "\n");
		}
	}
	return 0;
}
