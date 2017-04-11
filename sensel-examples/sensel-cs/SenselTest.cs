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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sensel
{
    class SenselTest
    {

        static void Main(string[] args)
        {
            IntPtr handle = new IntPtr(0);
            SenselFrame frame = new SenselFrame();
            SenselDeviceList list = new SenselDeviceList();
            list.num_devices = 0;
            Sensel.senselGetDeviceList(ref list);
            Console.WriteLine("Num Devices: " + list.num_devices);
            if (list.num_devices != 0)
            {
                Sensel.senselOpenDeviceByID(ref handle, list.devices[0].idx);
                Sensel.senselSetFrameContent(handle, 15);
                Sensel.senselAllocateFrameData(handle, frame);
                Sensel.senselStartScanning(handle);
            }
            if (handle.ToInt64() != 0)
            {
                for (int c = 0; c < 500; c++)
                {
                    Int32 num_frames = 0;
                    Sensel.senselReadSensor(handle);
                    Sensel.senselGetNumAvailableFrames(handle, ref num_frames);
                    for (int f = 0; f < num_frames; f++)
                    {
                        Sensel.senselGetFrame(handle, frame);
                        Console.WriteLine("Num Contacts: " + frame.n_contacts);
                        for (int i = 0; i < frame.n_contacts; i++)
                        {
                            Console.WriteLine("Contact ID: " + frame.contacts[i].id);
                        }
                        String forces = "Forces: ";
                        String labels = "Labels: ";
                        for(int i = 0; i < 10; i++)
                        {
                            forces += "[" + frame.force_array[i] + "]";
                            labels += "[" + frame.labels_array[i] + "]";
                        }
                        Console.WriteLine(forces);
                        Console.WriteLine(labels);
                    }
                }

                Sensel.senselStopScanning(handle);
                Sensel.senselClose(handle);
            }
        }
    }
}
