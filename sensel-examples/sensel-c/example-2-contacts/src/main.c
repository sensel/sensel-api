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
#include "sensel.h"
#include "sensel_device.h"

volatile sig_atomic_t ctrl_c_requested = false;

void handle_ctrl_c(int sig)
{
	ctrl_c_requested = true;
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
		return 0;
	}

	signal(SIGINT, handle_ctrl_c);
	fprintf(stdout, "Press Control-C to stop...\n");

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

	while (!ctrl_c_requested) {
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
					fprintf(stdout, "\nNum Contacts: %d\n", frame[i]->n_contacts);
				}
				for (int c = 0; c < frame[i]->n_contacts; c++)
				{
					unsigned int state = frame[i]->contacts[c].state;
					char* statestr =
						state == CONTACT_INVALID ? "CONTACT_INVALID" :
						state == CONTACT_START ? "CONTACT_START" :
						state == CONTACT_MOVE ? "CONTACT_MOVE" :
						state == CONTACT_END ? "CONTACT_END" :
						"UNKNOWN_CONTACT_STATE";
					fprintf(stdout, "Contact ID: %d    State: %s\n", frame[i]->contacts[c].id, statestr);

					if (state == CONTACT_START) {
						senselSetLEDBrightness(handle[i], frame[i]->contacts[c].id, 100);
					}
					else if ( state == CONTACT_END ) {
						senselSetLEDBrightness(handle[i], frame[i]->contacts[c].id, 0);
					}
				}
			}
		}
		Sleep(1);
	}
	return 0;
}
