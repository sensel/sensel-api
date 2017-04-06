
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensel.h"
#include "sensel_device.h"

#define TEST_SCAN_NUM_LOOPS 500

int main(int argc, char **argv)
{
	SENSEL_HANDLE handle = NULL;
	SenselDeviceList list;
	senselGetDeviceList(&list);
	if (!list.num_devices)
	{
		fprintf(stdout, "No device found\n");
		return 0;
	}
	senselOpenDeviceByID(&handle, list.devices[0].idx);
	SenselFrameData *frame = NULL;
	senselSetFrameContent(handle, 15);
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
		}
	}
	return 0;
}
