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

// serial.c: Windows serial communication

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <setupapi.h>
#include <devpkey.h>
#include "sensel_serial.h"
#include "sensel_register.h"
#include "sensel_register_map.h"
#include "sensel_device_vidpid.h"

#define SENSEL_COM_PORT_PREFIX "\\\\.\\"

unsigned char devices_scanned = false;
SenselDeviceList devlist;

static
unsigned char senselSerialOpen2(SenselSerialHandle *data, char* com_port)
{
  DCB           dcb;
  COMMTIMEOUTS  timeouts;
  BOOL          fSuccess;
  char          full_port_name[100];
  char          magic[7];

  magic[6] = '\0';

  strcpy(full_port_name, SENSEL_COM_PORT_PREFIX);
  strcat(full_port_name, com_port);

  data->serial_handle = CreateFileA(full_port_name,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,                            // must be opened with exclusive-access
                                    NULL,                         // no security attributes
                                    OPEN_EXISTING,                // must use OPEN_EXISTING
                                    FILE_ATTRIBUTE_NORMAL,        // not overlapped I/O
                                    NULL                          // hTemplate must be NULL for comm devices
                                    );

  if (data->serial_handle == INVALID_HANDLE_VALUE)
  {
    // Handle the error.
    printf ("CreateFile failed with error %d.\n", GetLastError());
    return false;
  }

  // Build on the current configuration, and skip setting the size
  // of the input and output buffers with SetupComm.
  fSuccess = GetCommState(data->serial_handle, &dcb);
  if (!fSuccess)
  {
    // Handle the error.
    printf ("GetCommState failed with error %d.\n", GetLastError());
    senselSerialClose(data);
    return false;
  }

  // Fill in DCB: 115,200 bps, 8 data bits, no parity, and 1 stop bit.
  dcb.BaudRate  = CBR_115200;    // CBR_9600;  // set the baud rate
  dcb.ByteSize  = 8;             // data size, xmit, and rcv
  dcb.Parity    = NOPARITY;      // no parity bit
  dcb.StopBits  = ONESTOPBIT;    // one stop bit

  // The following settings are probably not necessary
  dcb.DCBlength       = sizeof(dcb);
  dcb.fBinary         = TRUE;
  dcb.fDtrControl     = DTR_CONTROL_DISABLE;
  dcb.fRtsControl     = RTS_CONTROL_DISABLE;
  dcb.fOutxCtsFlow    = FALSE;
  dcb.fOutxDsrFlow    = FALSE;
  dcb.fDsrSensitivity = FALSE;
  dcb.fAbortOnError   = TRUE;

  fSuccess = SetCommState(data->serial_handle, &dcb);
  if (!fSuccess)
  {
    // Handle the error.
    printf("SetCommState failed with error %d.\n", GetLastError());
    senselSerialClose(data);
    return false;
  }

  GetCommTimeouts(data->serial_handle, &timeouts);

  timeouts.ReadIntervalTimeout        = 50; // If 50ms elapses after the last byte read, the read function times out
  timeouts.ReadTotalTimeoutConstant   = 50; // For each byte that we try to read, allow a maximum of 50ms...
  timeouts.ReadTotalTimeoutMultiplier = 10; // Plus a constant delay of 10ms
  timeouts.WriteTotalTimeoutConstant  = 50; // For each byte that we try to write, allow a maximum of 50ms...
  timeouts.WriteTotalTimeoutMultiplier= 10; // Plus a constant delay of 10ms

  if(!SetCommTimeouts(data->serial_handle, &timeouts))
  {
    printf("SetCommTimeouts failed.");
  }

  printf("Serial port %s successfully connected.\n", com_port);

  // Flush out any characters that might be in the serial receive buffer
  senselSerialFlushInput(data);

  // Check for the magic number
  if(_senselReadReg(NULL, data, 0x00, SENSEL_MAGIC_LEN, (unsigned char*)magic) == SENSEL_OK)
  {
    printf("Magic: %s\n", magic);
    if(strcmp(magic, SENSEL_MAGIC) == 0)
    {
      printf("Found sensor!\n");
      return true;
    }
    else
    {
      printf("Invalid magic!\n");
    }
  }
  else
  {
    printf("Timeout on read!\n");
  }

  senselSerialClose(data);
  return false;
}

static unsigned char ComPortNames(SenselSerialHandle *data)
{
  unsigned        index;
  unsigned        devtypeidx;
  HDEVINFO        hDevInfo;
  SP_DEVINFO_DATA DeviceInfoData;
  TCHAR           HardwareID[1024];

  // List all connected USB devices
  hDevInfo = SetupDiGetClassDevs(NULL, TEXT("USB"), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
  for (index = 0; ; index++) {
    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    if (!SetupDiEnumDeviceInfo(hDevInfo, index, &DeviceInfoData)) {
      return false;     // no match
    }

    SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (BYTE*)HardwareID, sizeof(HardwareID), NULL);

    for (devtypeidx = 0; devtypeidx < NUM_SUPPORTED_DEVS; devtypeidx++)
    {
      if (_tcsstr(HardwareID, _T(supported_devices[devtypeidx].vidpidstr)))
      {
        HKEY  hKeyDevice = SetupDiOpenDevRegKey(hDevInfo, &DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        char  acValue[256];
        DWORD dwSize = sizeof(acValue);

        printf("Found device: %s\n", supported_devices[devtypeidx].name);
        RegQueryValueExW(hKeyDevice, L"PortName", NULL, NULL, (LPBYTE)acValue, &dwSize);
        RegCloseKey(hKeyDevice);

        if (dwSize < 255) {
          char com[128];
          for (unsigned int i = 0; i < dwSize + 1; i++) {
            com[i] = acValue[2 * i];
          }
          if (senselSerialOpen2(data, com))
          {
            return true;
          }
        }
      }
    }
  }
  return false;
}

unsigned char senselSerialScan(SenselDeviceList *list)
{
  unsigned int        index;
  unsigned int        devtypeidx;
  HDEVINFO            hDevInfo;
  SP_DEVINFO_DATA     DeviceInfoData;
  TCHAR               HardwareID[1024];
  SenselSerialHandle  serial;
  SenselStatus        status      = SENSEL_OK;
  unsigned char       found_sensor = 0;
  unsigned char       num_devices = 0;

  memset(list, 0, sizeof(SenselDeviceList));
  // List all connected USB devices
  hDevInfo = SetupDiGetClassDevs(NULL, TEXT("USB"), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
  for (index = 0; ; index++)
  {
    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    if (!SetupDiEnumDeviceInfo(hDevInfo, index, &DeviceInfoData)) {
      devices_scanned = true;
      memcpy(list, &devlist, sizeof(SenselDeviceList));
      return num_devices;
    }

    SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, NULL, (BYTE*)HardwareID, sizeof(HardwareID), NULL);

    for (devtypeidx = 0; devtypeidx < NUM_SUPPORTED_DEVS; devtypeidx++)
    {
      if (_tcsstr(HardwareID, _T(supported_devices[devtypeidx].vidpidstr)))
      {
        HKEY hKeyDevice = SetupDiOpenDevRegKey(hDevInfo, &DeviceInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

        char  acValue[256];
        DWORD dwSize = sizeof(acValue);

        RegQueryValueExW(hKeyDevice, L"PortName", NULL, NULL, (LPBYTE)acValue, &dwSize);
        RegCloseKey(hKeyDevice);

        if (dwSize < 255)
        {
          char com[128];
          for (unsigned int i = 0; i < dwSize + 1; i++) {
            com[i] = acValue[2 * i];
          }

          found_sensor = senselSerialOpen2(&serial, com);
          if (found_sensor)
          {
            SenselDeviceID *devid = &devlist.devices[num_devices];
            unsigned int num_chars;

            status = _senselReadRegVS(NULL, &serial, SENSEL_REG_DEVICE_SERIAL_NUMBER, sizeof(devid->serial_num),
                devid->serial_num, &num_chars);
            if (status != SENSEL_OK)
              continue;

            // TODO: This is an issue in the firmware code where although the firmware reports a 16 byte long serial, only 13
            //       of them are actually valid. As a consequence, scan through the string and replace 0xFF with 0.
            for (unsigned int i = 0; i < num_chars; i++)
              devid->serial_num[i] = (devid->serial_num[i] == 0xFF) ? 0 : devid->serial_num[i];

            devid->idx = num_devices;
            devid->serial_num[num_chars] = 0;
            strncpy((char*)devid->com_port, com, sizeof(devid->com_port));
            devlist.num_devices = ++num_devices;
          }
          senselSerialClose(&serial);
          if (num_devices == SENSEL_MAX_DEVICES)
            break;
        }
      }
    }
  }

  return num_devices;
}

unsigned char senselSerialOpen(SenselSerialHandle *data, char* com_port)
{
  if(com_port != NULL)
  {
    return senselSerialOpen2(data, com_port);
  }
  return ComPortNames(data);
}

void senselSerialFlushInput(SenselSerialHandle *data)
{
  int           bytes_read = 0;
  int           byte_total = 0;
  unsigned char buf[128];

  do
  {
    //Try reading a byte
    bytes_read = senselSerialReadAvailable(data, buf, 128);
    if(bytes_read > 0) // We read a byte!
      byte_total += bytes_read;
  }
  while(bytes_read > 0);

  printf("Cleared %d bytes from buffer!\n", byte_total);
}

unsigned char senselSerialOpenDeviceByID(SenselSerialHandle *data, unsigned char idx)
{
  int i;

  if (!devices_scanned)
  {
    printf("senselSerialOpenDeviceByID. Need to call senselGetDeviceList first\n");
    return false;
  }

  for (i = 0; i < devlist.num_devices; i++)
  {
    if (devlist.devices[i].idx == idx)
      return senselSerialOpen2(data, (char*)devlist.devices[i].com_port);
  }
  return false;
}

unsigned char senselSerialOpenDeviceBySerialNum(SenselSerialHandle *data, char *serial_num)
{
  int i;

  if (!devices_scanned)
  {
    printf("senselSerialOpenDeviceBySerialNum. Need to call senselGetDeviceList first\n");
    return false;
  }

  for (i = 0; i < devlist.num_devices; i++)
  {
    if (!strcmp((char *)serial_num, (char *)devlist.devices[i].serial_num))
      return senselSerialOpen2(data, (char*)devlist.devices[i].com_port);
  }
  return false;
}

unsigned char senselSerialOpenDeviceByComPort(SenselSerialHandle *data, char *com_port)
{
  int i;

  if (!devices_scanned)
  {
    printf("senselSerialOpenByComPort. Need to call senselGetDeviceList first\n");
    return false;
  }

  for (i = 0; i < devlist.num_devices; i++)
  {
    if (!strcmp((char*)com_port, (char*)devlist.devices[i].com_port))
      return senselSerialOpen2(data, (char*)devlist.devices[i].com_port);
  }
  return false;
}

unsigned char senselSerialWrite(SenselSerialHandle *data, unsigned char* buf, int bufLen)
{
  DWORD dwBytesWritten = 0;

  if(!WriteFile(data->serial_handle, buf, bufLen, &dwBytesWritten, NULL))
  {
    printf("error writing to output buffer");
    return false;
  }

  if(dwBytesWritten != bufLen)
  {
    printf("error only %d out of %d bytes written", dwBytesWritten, bufLen);
    return false;
  }

  //printf("Data written to write buffer is %s", buffWrite);
  return true;
}

// Reads as many bytes as available, up to bufLen, returns number of bytes read or -1 on error
int senselSerialReadAvailable(SenselSerialHandle *data, unsigned char* buf, int bufLen)
{
  //buffRead = 0;
  DWORD dwBytesRead = 0;

  if (!ReadFile(data->serial_handle, buf, bufLen, &dwBytesRead, NULL))
  {
    printf("error reading from input buffer");
    return -1;
  }

  return dwBytesRead;
}

// Reads a requested number of bytes, returns true on success, false on failure
unsigned char senselSerialReadBytes(SenselSerialHandle *data, unsigned char* buf, int bufLen)
{
  int max_attempts  = 3;
  int attempts      = 0;

  do
  {
    int bytes_read = senselSerialReadAvailable(data, buf, bufLen);
    if(bytes_read == -1) return false;
    buf += bytes_read;
    bufLen -= bytes_read;

    if(bytes_read == 0)
      attempts++;

    if(attempts >= max_attempts)
    {
      return false;
    }
  }
  while(bufLen > 0);

  return true;
}

int senselSerialGetAvailable(SenselSerialHandle *data)
{
  COMSTAT comStatStruct;

  if(!ClearCommError(data->serial_handle, 0, &comStatStruct))
  {
    printf("SENSEL ERROR: Could not get number of available bytes on serial port.\n");
    return 0;
  }

  //printf("Num Chars: %d\n", comStatStruct.cbInQue);

  return comStatStruct.cbInQue;
}

void senselSerialClose(SenselSerialHandle *data)
{
  if(data->serial_handle != INVALID_HANDLE_VALUE)
  {
    CloseHandle(data->serial_handle);
    data->serial_handle = INVALID_HANDLE_VALUE;
  }
}
