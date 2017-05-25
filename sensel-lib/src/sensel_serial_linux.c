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

#include "sensel.h"
#include "sensel_device.h"
#include "sensel_serial.h"
#include "sensel_register.h"
#include "sensel_register_map.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <dirent.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define SENSEL_SERIAL_DIR "/dev/"

//TODO: Add a separate for flushing data
#define SENSEL_SERIAL_TIMEOUT_SEC       0
#define SENSEL_SERIAL_TIMEOUT_US        (500 * 1000) //500 ms

//TODO: This list does not get updated on disconnects
unsigned char    devices_scanned = 0;
SenselDeviceList devlist;

unsigned char senselSerialWrite(SenselSerialHandle *data, unsigned char *buf, int buf_len)
{
  int wr = write(data->serial_fd, buf, buf_len);

  //THIS CAUSES READ FAILURE IN MAC OSX!!! tcflush(data->serial_fd, TCOFLUSH);

  if(wr == -1)
  {
    //perror("Write Error:");
    return 0;
  }
  return 1;
}

int senselSerialReadAvailable(SenselSerialHandle *data, unsigned char *buf, int buf_len)
{
  fd_set read_fds;

  FD_ZERO(&read_fds);
  FD_SET(data->serial_fd, &read_fds);

  //select() changes timeout value, so we need this on every call to readAvailable
  struct timeval timeout;
  timeout.tv_sec  = SENSEL_SERIAL_TIMEOUT_SEC;
  timeout.tv_usec = SENSEL_SERIAL_TIMEOUT_US;

  //select() uses fd + 1
  int ret = select(data->serial_fd + 1, &read_fds, NULL, NULL, &timeout);

  if(ret == -1) //Select error
  {
    perror("Error on select()");
    return -1;
  }
  else if (ret > 0) //We have bytes to read!
  {
    ret = read(data->serial_fd, buf, buf_len);

    if(ret < 0)
      perror("read returned -1");

    return ret;
  }
  else //No data available after timeout
  {
    printf("[fd:%d] select() timedout with no data, ret=%d\n", data->serial_fd, ret);
    return 0;
  }
}

unsigned char senselSerialReadBytes(SenselSerialHandle *data, unsigned char *buf, int buf_len)
{
  int max_attempts    = 1;
  int failed_attempts = 0;
  int bytes_left      = buf_len;

  do
  {
    int bytes_read = senselSerialReadAvailable(data, buf, bytes_left);

    if(bytes_read <= 0)
    {
      failed_attempts++;
    }
    else
    {
      buf += bytes_read;
      bytes_left -= bytes_read;
    }

    if(failed_attempts >= max_attempts)
    {
      return 0;
    }
  }
  while(bytes_left > 0);

  return 1;
}

// TODO: I have not tested this on Linux!!!
int senselSerialGetAvailable(SenselSerialHandle *data)
{
  int bytes_avail;

  ioctl(data->serial_fd, FIONREAD, &bytes_avail);

  return bytes_avail;
}

void senselSerialFlushInput(SenselSerialHandle *data)
{
  int           bytes_read = 0;
  int           byte_total = 0;
  unsigned char buf[128];

  //Read while there are bytes available
  do
  {
    bytes_read = senselSerialReadAvailable(data, buf, 128);
    byte_total += bytes_read;
  }
  while(bytes_read > 0);
}

unsigned char senselSerialOpen2(SenselSerialHandle *data, char* file_name)
{
  char magic[7];

  magic[6] = '\0';

  data->serial_fd = open(file_name, O_RDWR | O_NONBLOCK | O_NOCTTY);

  if(data->serial_fd == -1)
  {
    return 0;
  }

  struct termios options;
  tcgetattr(data->serial_fd, &options);

  options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
      | INLCR | IGNCR | ICRNL | IXON);
  options.c_oflag &= ~OPOST;
  options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  options.c_cflag &= ~(CSIZE | PARENB);
  options.c_cflag |= CS8;

  cfsetispeed(&options, B115200);
  cfsetospeed(&options, B115200);
  tcsetattr(data->serial_fd, TCSANOW, &options);

  //senselSerialFlushInput(data);

  if(_senselReadReg(NULL, data, 0x00, SENSEL_MAGIC_LEN, (unsigned char *)magic) == SENSEL_OK)
  {
    if(strcmp(magic, SENSEL_MAGIC) == 0)
    {
      return 1;
    }
  }

  close(data->serial_fd);
  data->serial_fd = -1;
  return 0;
}

unsigned char senselSerialOpen(SenselSerialHandle *data, char* com_port)
{
  DIR           *d;
  struct dirent *dir;
  char          file_name[100];
  unsigned char found_sensor = 0;

  if(com_port != NULL)
  {
    return senselSerialOpen2(data, com_port);
  }

  d = opendir(SENSEL_SERIAL_DIR);
  if(!d)
  {
    printf("Could not open %s directory.", SENSEL_SERIAL_DIR);
    return 0;
  }

  while((dir = readdir(d)) != 0)
  {
    strcpy(file_name, SENSEL_SERIAL_DIR);

    if(strstr(dir->d_name, "morph") ||
       strstr(dir->d_name, "squirt") ||
       strstr(dir->d_name, "ttyACM") ||
       strstr(dir->d_name, "tty.usbmodem") ||
       strstr(dir->d_name, "cu.usbmodem"))
    {
      strcat(file_name, dir->d_name);

      found_sensor = senselSerialOpen2(data, file_name);
      if(found_sensor)
      {
        break;
      }
    }
  }
  closedir(d);

  return found_sensor;
}

unsigned char senselSerialOpenDeviceByID(SenselSerialHandle *data, unsigned char idx)
{
  int i;

  if(!devices_scanned)
  {
    printf("senselSerialOpenDeviceByID. Need to call senselGetDeviceList first\n");
    return 0;
  }

  for (i = 0; i < devlist.num_devices; i++)
  {
    if (devlist.devices[i].idx == idx)
      return senselSerialOpen2(data, (char*)devlist.devices[i].com_port);
  }
  return 0;
}

unsigned char senselSerialOpenDeviceBySerialNum(SenselSerialHandle *data, char *serial_num)
{
  int i;

  if(!devices_scanned)
  {
    printf("senselSerialOpenDeviceBySerialNum. Need to call senselGetDeviceList first\n");
    return 0;
  }

  for (i = 0; i < devlist.num_devices; i++)
  {
    if (!strcmp((char *)serial_num, (char *)devlist.devices[i].serial_num))
      return senselSerialOpen2(data, (char*)devlist.devices[i].com_port);
  }
  return 0;
}

unsigned char senselSerialOpenDeviceByComPort(SenselSerialHandle *data, char *com_port)
{
  int i;

  if(!devices_scanned)
  {
    printf("senselSerialOpenByComPort. Need to call senselGetDeviceList first\n");
    return 0;
  }

  for (i = 0; i < devlist.num_devices; i++)
  {
    if (!strcmp((char*)com_port, (char*)devlist.devices[i].com_port))
      return senselSerialOpen2(data, (char*)devlist.devices[i].com_port);
  }
  return 0;
}

unsigned char senselSerialScan(SenselDeviceList *list)
{
  SenselStatus        status;
  SenselSerialHandle  serial;
  DIR                 *d;
  struct dirent       *dir;
  char                file_name[128];
  unsigned char       found_sensor    = 0;
  unsigned char       num_devices     = 0;

  d = opendir(SENSEL_SERIAL_DIR);
  if(!d)
  {
    printf("Could not open %s directory.", SENSEL_SERIAL_DIR);
    return false;
  }

  memset(&devlist, 0, sizeof(SenselDeviceList));

  while((dir = readdir(d)) != 0)
  {
    strcpy(file_name, SENSEL_SERIAL_DIR);

    if(strstr(dir->d_name, "morph")  ||
       strstr(dir->d_name, "squirt") ||
       strstr(dir->d_name, "ttyACM") ||
       strstr(dir->d_name, "tty.usbmodem") ||
       strstr(dir->d_name, "cu.usbmodem"))
    {
      printf("Found device: %s\n", dir->d_name);
      strcat(file_name, dir->d_name);
      found_sensor = senselSerialOpen2(&serial, file_name);
      if (found_sensor)
      {
        SenselDeviceID *devid = &devlist.devices[num_devices];
        unsigned int num_chars;

        status = _senselReadRegVS(NULL, &serial, SENSEL_REG_DEVICE_SERIAL_NUMBER, sizeof(devid->serial_num),
                                  (unsigned char*)devid->serial_num, &num_chars);
        if (status != SENSEL_OK)
        {
          senselSerialClose(&serial);
          continue;
        }

        // TODO: This is an issue in the firmware code where although the firmware reports a 16 byte long serial, only 13
        //       of them are actually valid. As a consequence, scan through the string and replace 0xFF with 0.
        for (int i = 0; i < num_chars; i++)
          devid->serial_num[i] = (devid->serial_num[i]== 0xFF) ? 0 : devid->serial_num[i];

        devid->idx = num_devices;
        devid->serial_num[num_chars] = 0;
        strncpy((char*)devid->com_port, file_name, sizeof(devid->com_port));
        devlist.num_devices = ++num_devices;
      }
      senselSerialClose(&serial);
      if (num_devices == SENSEL_MAX_DEVICES)
        break;
    }
  }

  devices_scanned = true;
  memcpy(list, &devlist, sizeof(SenselDeviceList));

  closedir(d);
  return num_devices;
}

void senselSerialClose(SenselSerialHandle *data)
{
  if(data->serial_fd != -1)
  {
    close(data->serial_fd);
    data->serial_fd = -1;
  }
}
