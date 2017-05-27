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
    class SenselContacts
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
            sensel_device.SetFrameContent(SenselDevice.FRAME_CONTENT_CONTACTS_MASK);
            sensel_device.StartScanning();

            Console.WriteLine("Press any key to exit");
            while (!Console.KeyAvailable) { 
                sensel_device.ReadSensor();
                int num_frames = sensel_device.GetNumAvailableFrames();
                for (int f = 0; f < num_frames; f++)
                {
                    SenselFrame frame = sensel_device.GetFrame();
                    if (frame.n_contacts > 0)
                    {
                        Console.WriteLine("\nNum Contacts: " + frame.n_contacts);
                        for (int i = 0; i < frame.n_contacts; i++)
                        {
                            Console.WriteLine("Contact ID: " + frame.contacts[i].id);
                            if (frame.contacts[i].state == (int)SenselContactState.CONTACT_START)
                                sensel_device.SetLEDBrightness(frame.contacts[i].id, 100);
                            if (frame.contacts[i].state == (int)SenselContactState.CONTACT_END)
                                sensel_device.SetLEDBrightness(frame.contacts[i].id, 0);
                        }
                    }
                }
            }
            byte num_leds = sensel_device.GetNumAvailableLEDs();
            for(int i = 0; i < num_leds; i++)
            {
                sensel_device.SetLEDBrightness((byte)i, 0);
            }
            sensel_device.StopScanning();
            sensel_device.Close();
        }
    }
}
