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
#include <signal.h>
#ifdef WIN32
	#include <windows.h>
#else
	#include <pthread.h>
#endif
#include "sensel.h"
#include "sensel_device.h"

static const char* CONTACT_STATE_STRING[] = { "CONTACT_INVALID","CONTACT_START", "CONTACT_MOVE", "CONTACT_END" };
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
	SENSEL_HANDLE handle[SENSEL_MAX_DEVICES] = { NULL };
	//List of all available Sensel devices
	SenselDeviceList list;
	//SenselFrame data that will hold the contacts
	SenselFrameData *frame[SENSEL_MAX_DEVICES] = { NULL };

	//Get a list of avaialble Sensel devices
	senselGetDeviceList(&list);
	if (list.num_devices == 0)
	{
		fprintf(stdout, "No device found\n");
		fprintf(stdout, "Press Enter to exit example\n");
		getchar();
		return 0;
	}

	//Open all Sensel devices by the id in the SenselDeviceList, handle initialized 
	for (int i = 0; i < list.num_devices; i++) {
		senselOpenDeviceByID(&handle[i], list.devices[i].idx);

		//Set the frame content to scan contact data
		senselSetFrameContent(handle[i], FRAME_CONTENT_CONTACTS_MASK);
		//Allocate a frame of data, must be done before reading frame data
		senselAllocateFrameData(handle[i], &frame[i]);
		//Start scanning the Sensel device
		senselStartScanning(handle[i]);
    }
    
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
		for (int i = 0; i < list.num_devices; i++) {
			senselReadSensor(handle[i]);
			//Get number of frames available in the data read from the sensor
			senselGetNumAvailableFrames(handle[i], &num_frames);
			for (unsigned int f = 0; f < num_frames; f++)
			{
				//Read one frame of data
				senselGetFrame(handle[i], frame[i]);
				//Print out contact data
				if (frame[i]->n_contacts > 0) {
					fprintf(stdout, "\nMorph %d Num Contacts: %d\n", i, frame[i]->n_contacts);
					for (int c = 0; c < frame[i]->n_contacts; c++)
					{
						unsigned int state = frame[i]->contacts[c].state;
						fprintf(stdout, "Contact ID: %d State: %s\n", frame[i]->contacts[c].id, CONTACT_STATE_STRING[state]);

						//Turn on LED for CONTACT_START
						if (state == CONTACT_START) {
							senselSetLEDBrightness(handle[i], frame[i]->contacts[c].id, 100);
						}
						//Turn off LED for CONTACT_END
						else if (state == CONTACT_END) {
							senselSetLEDBrightness(handle[i], frame[i]->contacts[c].id, 0);
						}
					}
				}
			}
		}
	}
	return 0;
}
