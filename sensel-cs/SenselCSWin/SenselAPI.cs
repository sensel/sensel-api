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

using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System;

namespace Sensel
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SenselDeviceID
    {
        public byte idx;
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 64)]
        public byte[] serial_num;
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 64)]
        public byte[] com_port;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SenselDeviceList
    {
        public byte num_devices;
        [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 16)]
        public SenselDeviceID[] devices;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SenselSensorInfo
    {
        public byte max_contacts;
        public UInt16 num_rows;
        public UInt16 num_cols;
        public float width;
        public float height;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SenselContact
    {
        public byte content_bit_mask;
        public byte id;
        public Int32 state;
        public float x_pos;
        public float y_pos;
        public float total_force;
        public float area;
        public float orientation;
        public float major_axis;
        public float minor_axis;
        public float delta_x;
        public float delta_y;
        public float delta_force;
        public float delta_area;
        public float min_x;
        public float min_y;
        public float max_x;
        public float max_y;
        public float peak_x;
        public float peak_y;
        public float peak_force;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SenselAccelData
    {
        public UInt32 x;
        public UInt32 y;
        public UInt32 z;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class SenselFrameData
    {
        public byte content_bit_mask;
        public Int32 lost_frame_count;
        public byte n_contacts;
        public IntPtr contacts;
        public IntPtr force_array;
        public IntPtr labels_array;
        public IntPtr accel_data;
    }


    [StructLayout(LayoutKind.Sequential)]
    public class SenselFrame
    {
        public byte content_bit_mask;
        public Int32 lost_frame_count;
        public byte n_contacts;
        public SenselContact[] contacts;
        public float[] force_array;
        public byte[] labels_array;
        public SenselAccelData accel_data;

        internal SenselFrameData frame_data;
    }

    public class SenselAPI
    {

        [DllImport("LibSensel")]
        public extern static int senselGetDeviceList(ref SenselDeviceList device_list);

        [DllImport("LibSensel")]
        public extern static int senselOpenDeviceByID(ref IntPtr handle, byte idx);

        [DllImport("LibSensel")]
        public extern static int senselClose(IntPtr handle);

        [DllImport("LibSensel")]
        public extern static int senselGetSensorInfo(IntPtr handle, ref SenselSensorInfo info);

        [DllImport("LibSensel")]
        private extern static int senselAllocateFrameData(IntPtr handle, ref SenselFrameData data);

        [DllImport("LibSensel")]
        public extern static int senselSetFrameContent(IntPtr handle, byte content);

        [DllImport("LibSensel")]
        public extern static int senselStartScanning(IntPtr handle);

        [DllImport("LibSensel")]
        public extern static int senselStopScanning(IntPtr handle);

        [DllImport("LibSensel")]
        public extern static int senselReadSensor(IntPtr handle);

        [DllImport("LibSensel")]
        public extern static int senselGetNumAvailableFrames(IntPtr handle, ref Int32 num_avail_frames);

        [DllImport("LibSensel")]
        private extern static int senselGetFrame(IntPtr handle, SenselFrameData data);


        public static int senselAllocateFrame(IntPtr handle, SenselFrame frame)
        {
            SenselSensorInfo info = new SenselSensorInfo();
            SenselAPI.senselGetSensorInfo(handle, ref info);
            frame.contacts = new SenselContact[info.max_contacts];
            for (int i = 0; i < info.max_contacts; i++)
                frame.contacts[i] = new SenselContact();
            frame.force_array = new float[info.num_rows * info.num_cols];
            frame.labels_array = new byte[info.num_rows * info.num_cols];
            frame.accel_data = new SenselAccelData();
            frame.frame_data = new SenselFrameData();

            SenselAPI.senselAllocateFrameData(handle, ref frame.frame_data);
            return 0;
        }

        public static int senselGetFrame(IntPtr handle, SenselFrame frame)
        {
            SenselAPI.senselGetFrame(handle, frame.frame_data);
            frame.n_contacts = frame.frame_data.n_contacts;
            long ptrIndex = (frame.frame_data.contacts).ToInt64();
            for (int i = 0; i < frame.frame_data.n_contacts; i++)
            {
                IntPtr cPtr = new IntPtr(ptrIndex);
                frame.contacts[i] = (SenselContact)Marshal.PtrToStructure(cPtr, typeof(SenselContact));
                ptrIndex += Marshal.SizeOf(typeof(SenselContact));
            }
            Marshal.Copy(frame.frame_data.force_array, frame.force_array, 0, frame.force_array.Length);
            Marshal.Copy(frame.frame_data.labels_array, frame.labels_array, 0, frame.labels_array.Length);
            frame.accel_data = (SenselAccelData)Marshal.PtrToStructure(frame.frame_data.accel_data, typeof(SenselAccelData));
            return 0;
        }
    }
}
