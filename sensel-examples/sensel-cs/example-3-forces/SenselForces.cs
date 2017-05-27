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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Sensel;

namespace SenselExamples
{
    class SenselForces
    {
        static void Main(string[] args)
        {
            SenselDeviceList list = SenselDevice.GetDeviceList();
            Console.WriteLine("Num Devices: " + list.num_devices);
            if (list.num_devices == 0)
            {
                Console.WriteLine("No devices found.");
                Console.WriteLine("Press any key to exit.");
                while (!Console.KeyAvailable) { }
                return;
            }

            SenselDevice sensel_device = new SenselDevice();
            sensel_device.OpenDeviceByID(list.devices[0].idx);
            sensel_device.SetFrameContent(SenselDevice.FRAME_CONTENT_PRESSURE_MASK);
            sensel_device.StartScanning();
            SenselSensorInfo sensor_info = sensel_device.GetSensorInfo();

            Console.WriteLine("Press any key to exit");
            while (!Console.KeyAvailable)
            {
                sensel_device.ReadSensor();
                int numFrames = sensel_device.GetNumAvailableFrames();
                for (int f = 0; f < numFrames; f++)
                {
                    SenselFrame frame = sensel_device.GetFrame();
                    float total_force = 0;
                    for(int i =0; i < sensor_info.num_cols* sensor_info.num_rows; i++)
                    {
                        total_force = total_force + frame.force_array[i];
                    }
                    Console.WriteLine("Total Force: " + total_force);
                }
            }
            sensel_device.StopScanning();
            sensel_device.Close();
        }
    }
}
