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
using System.Threading;
using System.Threading.Tasks;
using Sensel;

namespace SenselExamples
{
    class HelloSensel
    {
        static void Main(string[] args)
        {
            SenselDeviceList list = SenselDevice.GetDeviceList();
            Console.WriteLine("Num Devices: " + list.num_devices);
            if (list.num_devices != 0)
            {
                SenselDevice sensel_device = new SenselDevice();
                sensel_device.OpenDeviceByID(list.devices[0].idx);
                SenselSensorInfo sensor_info = sensel_device.GetSensorInfo();
                SenselFirmwareInfo fw_info = sensel_device.GetFirmwareInfo();
                Console.WriteLine("Sensel Device: " + System.Text.Encoding.Default.GetString(list.devices[0].serial_num));
                Console.WriteLine("Firmware Version: " + fw_info.fw_version_major + "." + fw_info.fw_version_minor + "." + fw_info.fw_version_build);
                Console.WriteLine("Width: " + sensor_info.width + "mm");
                Console.WriteLine("Height: " + sensor_info.height + "mm");
                Console.WriteLine("Cols: " + sensor_info.num_cols);
                Console.WriteLine("Rows: " + sensor_info.num_rows);
                sensel_device.Close();
            }
            Console.WriteLine("Press any key to exit.");
            while (!Console.KeyAvailable){}
        }
    }
}
