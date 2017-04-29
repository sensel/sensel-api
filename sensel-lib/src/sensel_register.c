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
#include "sensel.h"
#include "sensel_device.h"
#include "sensel_register.h"
#include "sensel_serial.h"

extern unsigned char _senselReadFrame(SenselDevice *device);

sensel_protocol_cmd_t    read_cmd     = {(DEFAULT_BOARD_ADDR | (1 << 7)), 0x00, 0x00, 0x00};
sensel_protocol_cmd_t    write_cmd    = { DEFAULT_BOARD_ADDR, 0x00, 0x00, 0x00};
sensel_protocol_cmd_vs_t write_cmd_vs = { DEFAULT_BOARD_ADDR, 0x00, 0x00, 0, 0x00, 0x00};

SenselStatus _senselReadReg(SENSEL_HANDLE handle, SenselSerialHandle *serial, unsigned char reg,
                            unsigned char size, unsigned char *buf)
{
  SenselDevice    *device   = (SenselDevice *)handle;
  unsigned char   ack;
  unsigned char   resp_checksum;
  unsigned short  resp_size;
  unsigned char   checksum  = 0;
  int             i;

  read_cmd.reg = reg;
  read_cmd.size = size;

  if(!senselSerialWrite(serial, (unsigned char *)&read_cmd, 3))
    return SENSEL_ERROR;

  if(!senselSerialReadBytes(serial, (unsigned char *)&ack, 1))
    return SENSEL_ERROR;

  if (device)
  {
    while(device->scan_mode == SCAN_MODE_ASYNC && ack == PT_ASYNC_DATA)
    {
      if(!_senselReadFrame(device))
      {
        printf("SENSEL ERROR: Error reading async frame.\n");
      }

      // Get the new ack
      if(!senselSerialReadBytes(serial, (unsigned char *)&ack, 1))
        return SENSEL_ERROR;
    }
  }

  if(ack != PT_READ_ACK)
  {
    return SENSEL_ERROR;
  }

  if (!senselSerialReadBytes(serial, (unsigned char *)&reg, 1))
    return SENSEL_ERROR;

  if(!senselSerialReadBytes(serial, (unsigned char*)&resp_size, 2))
    return SENSEL_ERROR;

  if(resp_size != size)
    return SENSEL_ERROR;

  if(!senselSerialReadBytes(serial, buf, size))
    return SENSEL_ERROR;

  if(!senselSerialReadBytes(serial, &resp_checksum, 1))
    return SENSEL_ERROR;

  for(i = 0; i < size; i++)
    checksum += buf[i];

  //printf("Checksums: (%d == %d)\n", checksum, resp_checksum);

  if (checksum != resp_checksum)
    return SENSEL_ERROR;

  return SENSEL_OK;
}

SenselStatus _senselWriteReg(SENSEL_HANDLE handle, SenselSerialHandle *serial, unsigned char reg,
                             unsigned char size, unsigned char *buf)
{
  SenselDevice  *device   = (SenselDevice *)handle;
  unsigned char ack;
  unsigned char checksum  = 0;
  int           i;

  write_cmd.reg = reg;
  write_cmd.size = size;

  for(i = 0; i < size; i++)
    checksum += buf[i];

  //Send write header
  if(!senselSerialWrite(serial, (unsigned char *)&write_cmd, 3))
    return SENSEL_ERROR;

  //Send data
  if(!senselSerialWrite(serial, buf, size))
    return SENSEL_ERROR;

  //Send checksum
  if(!senselSerialWrite(serial, &checksum, 1))
    return SENSEL_ERROR;

  if(!senselSerialReadBytes(serial, &ack, 1))
    return SENSEL_ERROR;

  if (!senselSerialReadBytes(serial, (unsigned char *)&reg, 1))
    return SENSEL_ERROR;

  if (device)
  {
    while(device->scan_mode == SCAN_MODE_ASYNC && ack == PT_ASYNC_DATA)
    {
      if(!_senselReadFrame(device))
      {
        printf("SENSEL ERROR: Error reading async frame.\n");
      }

      // Get the new ack
      if(!senselSerialReadBytes(serial, (unsigned char *)&ack, 1))
        return SENSEL_ERROR;
    }
  }

  if (ack != PT_WRITE_ACK)
    return SENSEL_ERROR;

  return SENSEL_OK;
}

SenselStatus _senselReadRegVS(SENSEL_HANDLE handle, SenselSerialHandle *serial, unsigned char reg,
                              unsigned int buf_size, unsigned char *buf, unsigned int *read_size)
{
  unsigned short  read_size_buf;
  unsigned char   ack[3];
  unsigned char   checksum = 0;
  unsigned char   resp_checksum;
  int             i;

  read_cmd.reg = reg;
  read_cmd.size = 0;

  if(!senselSerialWrite(serial, (unsigned char *)&(read_cmd), 3))
    return false;

  if (!senselSerialReadBytes(serial, ack, 3))
    printf("Unable to read RVS ack\n");

  if(!senselSerialReadBytes(serial, (unsigned char *)&(read_size_buf), 2))
    printf("Unable to read RVS size\n");

  if (buf_size < read_size_buf)
  {
    printf("RegVS: Provided buffer is too small\n");
    return SENSEL_ERROR;
  }

  senselSerialReadBytes(serial, buf, read_size_buf);

  if (!senselSerialReadBytes(serial, (unsigned char *)&(resp_checksum), 1))
    printf("Unable to read RVS checksum\n");

  for(i = 0; i < read_size_buf; i++)
    checksum += buf[i];

  if (checksum != resp_checksum)
    printf("Invalid checksum\n");

  if (read_size)
    *read_size = read_size_buf;

  return SENSEL_OK;
}

SenselStatus _senselWriteRegVS(SENSEL_HANDLE handle, SenselSerialHandle *serial, unsigned char reg,
                               unsigned int size, unsigned char *buf, unsigned int *write_size)
{
  unsigned char   ack;
  unsigned char   checksum    = 0;
  unsigned short  packet_size = 0;
  unsigned int    vs_index    = 0;
  int             i;

  write_cmd_vs.reg          = reg;
  write_cmd_vs.size         = 0;
  write_cmd_vs.header_size  = DEFAULT_VS_HEADER_SIZE;
  write_cmd_vs.vs_size      = size;

  unsigned char *cmd_vs_bytes = (unsigned char *)&write_cmd_vs;
  write_cmd_vs.checksum = cmd_vs_bytes[4]+cmd_vs_bytes[5]+cmd_vs_bytes[6]+cmd_vs_bytes[7];

  //Send write header
  if(!senselSerialWrite(serial, cmd_vs_bytes, 9))
    return SENSEL_ERROR;

  if(!senselSerialReadBytes(serial, &ack, 1))
    return SENSEL_ERROR;

  if (!senselSerialReadBytes(serial, &reg, 1))
    return SENSEL_ERROR;

  while(vs_index < size)
  {
    if(size-vs_index >= MAX_VS_PACKET_SIZE)
      packet_size = MAX_VS_PACKET_SIZE;
    else
      packet_size = size-vs_index;

    checksum = 0;
    for(i = 0; i < packet_size; i++)
      checksum += buf[vs_index+i];

    //Send packet size
    if(!senselSerialWrite(serial, (unsigned char *)&packet_size, 2))
      return SENSEL_ERROR;

    //Send data
    if(!senselSerialWrite(serial, &buf[vs_index], packet_size))
      return SENSEL_ERROR;

    //Send checksum
    if(!senselSerialWrite(serial, &checksum, 1))
      return SENSEL_ERROR;

    //Read packet ack
    if(!senselSerialReadBytes(serial, &ack, 1))
      return SENSEL_ERROR;

    if (ack != PT_WVS_ACK)
      return SENSEL_ERROR;

    vs_index = vs_index + packet_size;

    if (write_size)
      *write_size = vs_index;
  }

  return SENSEL_OK;
}
