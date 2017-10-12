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
#ifdef WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif
#include "sensel.h"
#include "sensel_device.h"

static bool enter_pressed = false;

#ifdef WIN32
DWORD WINAPI waitForEnter()
#else
void * waitForEnter()
#endif
{
    getchar();
    enter_pressed = true;
    return 0;
}

int main(int argc, char **argv)
{
	//Handle that references a Sensel device
	SENSEL_HANDLE handle = NULL;
	//List of all available Sensel devices
	SenselDeviceList list;
	//Sensor info from the Sensel device
	SenselSensorInfo sensor_info;
	//SenselFrame data that will hold the forces
    SenselFrameData *frame = NULL;
	//Total of all forces in SenselFrame force_array
	float total_force = 0.0f;

	//Get a list of avaialble Sensel devices
	senselGetDeviceList(&list);
	if (list.num_devices == 0)
	{
		fprintf(stdout, "No device found\n");
		fprintf(stdout, "Press Enter to exit example\n");
		getchar();
		return 0;
	}

	//Open a Sensel device by the id in the SenselDeviceList, handle initialized 
	senselOpenDeviceByID(&handle, list.devices[0].idx);
	//Get the sensor info
	senselGetSensorInfo(handle, &sensor_info);
	
	//Set the frame content to scan force data
	senselSetFrameContent(handle, FRAME_CONTENT_PRESSURE_MASK);
	//Allocate a frame of data, must be done before reading frame data
	senselAllocateFrameData(handle, &frame);
	//Start scanning the Sensel device
	senselStartScanning(handle);
    
    fprintf(stdout, "Press Enter to exit example\n");
    #ifdef WIN32
        HANDLE thread = CreateThread(NULL, 0, waitForEnter, NULL, 0, NULL);
    #else
        pthread_t thread;
        pthread_create(&thread, NULL, waitForEnter, NULL);
    #endif
    
    while (!enter_pressed)
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
			//Calculate the total force
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
