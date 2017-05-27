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

int main(int argc, char **argv)
{
	//Handle that references a Sensel device
	SENSEL_HANDLE handle = NULL;
	//List of all available Sensel devices
    SenselDeviceList list;
	//Firmware info from the Sensel device
	SenselFirmwareInfo fw_info;
	//Sensor info from the Sensel device
	SenselSensorInfo sensor_info;
    
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
	//Get the firmware info
	senselGetFirmwareInfo(handle, &fw_info);
	//Get the sensor info
	senselGetSensorInfo(handle, &sensor_info);

	fprintf(stdout, "\nSensel Device: %s\n" ,list.devices[0].serial_num );
	fprintf(stdout, "Firmware Version: %d.%d.%d\n", fw_info.fw_version_major, fw_info.fw_version_minor, fw_info.fw_version_build);
	fprintf(stdout, "Width: %fmm\n", sensor_info.width);
	fprintf(stdout, "Height: %fmm\n", sensor_info.height);
	fprintf(stdout, "Cols: %d\n", sensor_info.num_cols);
	fprintf(stdout, "Rows: %d\n", sensor_info.num_rows);
    
    fprintf(stdout, "Press Enter to exit example\n");
    getchar();
    
	return 0;
}
