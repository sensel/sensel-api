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

//TODO: Right now, anyone that uses the API has to allocate arrays for labels and force images, even if they're only using contacts. This should not be the case

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sensel.h"
#include "sensel_device.h"
#include "sensel_serial.h"
#include "sensel_register_map.h"
#include "sensel_register.h"

#ifdef SENSEL_PRESSURE
#include "sensel_decompress.h"
#endif //SENSEL_PRESSURE

#define PRINT_RECEIVED_CONTACTS 0
#define PRINT_BUFFERING_DEBUG   0

// NOTE: Care must be taken when using these macros because they double-evaluate x and y
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define FRAME_BUFFER_INITIAL_CAPACITY 256
#define DEFAULT_FRAME_CONTENT_CONTROL (FRAME_CONTENT_PRESSURE_MASK | FRAME_CONTENT_LABELS_MASK | FRAME_CONTENT_CONTACTS_MASK)

#define CHECK_FREE(x) if((x)) free((x))

extern sensel_protocol_cmd_t read_cmd;
extern sensel_protocol_cmd_t write_cmd;

SENSEL_API
SenselStatus WINAPI senselReadReg(SENSEL_HANDLE handle, unsigned char reg, unsigned char size, unsigned char *buf)
{
  SenselDevice *device = (SenselDevice*) handle;

  if (!device)
    return SENSEL_ERROR;

  return _senselReadReg(handle, &device->sensor_serial, reg, size, buf);
}

//TODO: For some reason, when you write invalid values, the software doesn't react to the error.
SENSEL_API
SenselStatus WINAPI senselWriteReg(SENSEL_HANDLE handle, unsigned char reg, unsigned char size, unsigned char *buf)
{
  SenselDevice *device = (SenselDevice*)handle;

  if (!device)
    return SENSEL_ERROR;

  return _senselWriteReg(handle, &device->sensor_serial, reg, size, buf);
}

// TODO: I believe this function is no longer compatible with the latest protocol.
// We need to add a read of the frame_type field and we need to read and verify the checksum!
SENSEL_API
SenselStatus WINAPI senselReadRegVS(SENSEL_HANDLE handle, unsigned char reg, unsigned int buf_size, unsigned char *buf, unsigned int *read_size)
{
  SenselDevice *device = (SenselDevice*)handle;

  if (!device)
    return SENSEL_ERROR;

  return _senselReadRegVS(handle, &device->sensor_serial, reg, buf_size, buf, read_size);
}

SENSEL_API
SenselStatus WINAPI senselWriteRegVS(SENSEL_HANDLE handle, unsigned char reg, unsigned int size, unsigned char *buf, unsigned int *write_size)
{
  SenselDevice *device = (SenselDevice*)handle;

  if (!device)
    return SENSEL_ERROR;

  return _senselWriteRegVS(handle, &device->sensor_serial, reg, size, buf, write_size);
}

static SenselStatus _senselGetPrvFirmwareInfo(SENSEL_HANDLE handle, SenselFirmwareInfo *fw_info)
{
  SenselStatus           status = SENSEL_OK;
  sensel_firmware_info_t prtcl_fw_info;

  status = senselReadReg(handle, SENSEL_REG_FW_VERSION_PROTOCOL, sizeof(sensel_firmware_info_t), (unsigned char*)&prtcl_fw_info);
  if (status != SENSEL_OK)
    return status;

  fw_info->fw_protocol_version = prtcl_fw_info.fw_protocol_version;
  fw_info->fw_version_major    = prtcl_fw_info.fw_version_major;
  fw_info->fw_version_minor    = prtcl_fw_info.fw_version_minor;
  fw_info->fw_version_build    = prtcl_fw_info.fw_version_build;
  fw_info->fw_version_release  = prtcl_fw_info.fw_version_release;
  fw_info->device_id           = prtcl_fw_info.device_id;
  fw_info->device_revision     = prtcl_fw_info.device_revision;

  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselGetFirmwareInfo(SENSEL_HANDLE handle, SenselFirmwareInfo *fw_info)
{
  SenselDevice *device = (SenselDevice*)handle;

  if (!device)
    return SENSEL_ERROR;

  *fw_info = device->fw_info;
  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselGetSensorInfo(SENSEL_HANDLE handle, SenselSensorInfo *info)
{
  SenselDevice *device = (SenselDevice *)handle;

  *info = device->sensor_info;
  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselAllocateFrameData(SENSEL_HANDLE handle, SenselFrameData **data)
{
  SenselDevice    *device = (SenselDevice *)handle;
  SenselFrameData *f;

  if (!device)
    return SENSEL_ERROR;

  f = calloc(1, sizeof(SenselFrameData));

  *data = NULL;

  if (!f)
    return SENSEL_ERROR;

  f->force_array = (float*)malloc(device->sensor_info.num_rows * device->sensor_info.num_cols * sizeof(float));
  if (!f->force_array)
    return SENSEL_ERROR;

  f->labels_array = malloc(device->sensor_info.num_rows * device->sensor_info.num_cols * sizeof(label_t));
  if (!f->labels_array)
    return SENSEL_ERROR;

  f->contacts = malloc(device->sensor_info.max_contacts * sizeof(SenselContact));
  if (!f->contacts)
    return SENSEL_ERROR;

  f->accel_data = malloc(sizeof(SenselAccelData));
  if (!f->accel_data)
    return SENSEL_ERROR;

  *data = f;

  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselFreeFrameData(SENSEL_HANDLE handle, SenselFrameData *data)
{
  if (!data)
    return SENSEL_ERROR;

  CHECK_FREE(data->force_array);
  CHECK_FREE(data->labels_array);
  CHECK_FREE(data->contacts);
  CHECK_FREE(data->accel_data);

  free(data);

  return SENSEL_OK;
}

static SenselStatus _senselGetSensorMaxContacts(SENSEL_HANDLE handle, unsigned char *max_contacts)
{
  return senselReadReg(handle, SENSEL_REG_CONTACTS_MAX_COUNT, 1, max_contacts);
}

static SenselStatus _senselGetSensorNumRows(SENSEL_HANDLE handle, unsigned short *num_rows)
{
  return senselReadReg(handle, SENSEL_REG_SENSOR_NUM_ROWS, 2, (unsigned char*)num_rows);
}

static SenselStatus _senselGetSensorNumCols(SENSEL_HANDLE handle, unsigned short *num_cols)
{
  return senselReadReg(handle, SENSEL_REG_SENSOR_NUM_COLS, 2, (unsigned char*)num_cols);
}

static SenselStatus _senselGetDimsValueScale(SENSEL_HANDLE handle, float *scale)
{
  SenselStatus  status;
  unsigned char reg;

  status = senselReadReg(handle, SENSEL_REG_UNIT_SHIFT_DIMS, 1, &reg);
  if (status != SENSEL_OK)
    return status;

  *scale = (float)(1 << reg);
  return SENSEL_OK;
}

static SenselStatus _senselGetForceValueScale(SENSEL_HANDLE handle, float *scale)
{
  SenselStatus  status;
  unsigned char reg;

  status = senselReadReg(handle, SENSEL_REG_UNIT_SHIFT_FORCE, 1, &reg);
  if (status != SENSEL_OK)
    return status;

  *scale = (float)(1 << reg);
  return SENSEL_OK;
}

static SenselStatus _senselGetAngleValueScale(SENSEL_HANDLE handle, float *scale)
{
  SenselStatus  status;
  unsigned char reg;

  status = senselReadReg(handle, SENSEL_REG_UNIT_SHIFT_ANGLE, 1, &reg);
  if (status != SENSEL_OK)
    return status;

  *scale = (float)(1 << reg);
  return SENSEL_OK;
}

static SenselStatus _senselGetAreaValueScale(SENSEL_HANDLE handle, float *scale)
{
  SenselStatus  status;
  unsigned char reg;

  status = senselReadReg(handle, SENSEL_REG_UNIT_SHIFT_AREA, 1, &reg);
  if (status != SENSEL_OK)
    return status;

  *scale = (float)(1 << reg);
  return SENSEL_OK;
}

static SenselStatus _senselGetSensorWidthUM(SENSEL_HANDLE handle, unsigned int *width)
{
	return senselReadReg(handle, SENSEL_REG_SENSOR_ACTIVE_AREA_WIDTH_UM, 4, (unsigned char*)width);
}

static SenselStatus _senselGetSensorHeightUM(SENSEL_HANDLE handle, unsigned int *height)
{
	return senselReadReg(handle, SENSEL_REG_SENSOR_ACTIVE_AREA_HEIGHT_UM, 4, (unsigned char*)height);
}

SENSEL_API
SenselStatus WINAPI senselGetPowerButtonPressed(SENSEL_HANDLE handle, unsigned char *pressed)
{
  return senselReadReg(handle, SENSEL_REG_POWER_BUTTON_PRESSED, 1, pressed);
}

SENSEL_API
SenselStatus WINAPI senselGetNumAvailableLEDs(SENSEL_HANDLE handle, unsigned char *num_leds)
{
  return senselReadReg(handle, SENSEL_REG_LED_COUNT, 1, num_leds);
}

SENSEL_API
SenselStatus WINAPI senselGetMaxLEDBrightness(SENSEL_HANDLE handle, unsigned short *max_brightness)
{
  return senselReadReg(handle, SENSEL_REG_LED_BRIGHTNESS_MAX, 2, (unsigned char*)max_brightness);
}

SENSEL_API
SenselStatus WINAPI senselSetLEDBrightness(SENSEL_HANDLE handle, unsigned char led_id, unsigned short brightness)
{
  SenselDevice *device = (SenselDevice*)handle;

  if (!device->led_array)
    return SENSEL_ERROR;

  if (led_id >= device->num_leds)
    return SENSEL_ERROR;
  if (brightness > device->max_led_brightness)
    return SENSEL_ERROR;
  if (device->led_reg_size == 1)
  {
    unsigned char *led_array = (unsigned char*)device->led_array;
    led_array[led_id] = (unsigned char)brightness;
    return senselWriteRegVS(handle, SENSEL_REG_LED_BRIGHTNESS, device->num_leds * device->led_reg_size, led_array, NULL);
  }
  else
  {
    unsigned short *led_array = (unsigned short*)device->led_array;
    led_array[led_id] = brightness;
    return senselWriteRegVS(handle, SENSEL_REG_LED_BRIGHTNESS, device->num_leds * device->led_reg_size, (unsigned char*)led_array, NULL);
  }
  return SENSEL_OK;
}

SENSEL_API
SenselStatus _senselGetAllLEDBrightness(SENSEL_HANDLE handle)
{
  SenselDevice *device = (SenselDevice*)handle;

  if (!device->led_array)
    return SENSEL_ERROR;

  return senselReadRegVS(handle, SENSEL_REG_LED_BRIGHTNESS, device->num_leds * device->led_reg_size,
                         (unsigned char*)device->led_array, NULL);
}

SENSEL_API
SenselStatus WINAPI senselGetLEDBrightness(SENSEL_HANDLE handle, unsigned char led_id, unsigned short *brightness)
{
  SenselDevice *device = (SenselDevice*)handle;

  if (!device->led_array)
    return SENSEL_ERROR;

  if (led_id >= device->num_leds)
    return SENSEL_ERROR;
  if (device->led_reg_size == 1)
  {
    unsigned char *led_array = (unsigned char*)device->led_array;
    *brightness = (unsigned short)led_array[led_id];
    return SENSEL_OK;
  }
  else
  {
    unsigned short *led_array = (unsigned short*)device->led_array;
    *brightness = led_array[led_id];
    return SENSEL_OK;
  }
  return SENSEL_OK;
}

static SenselStatus _senselGetLEDRegSize(SENSEL_HANDLE handle, unsigned char *reg_size)
{
  return senselReadReg(handle, SENSEL_REG_LED_BRIGHTNESS_SIZE, 1, reg_size);
}

#ifdef SENSEL_PRESSURE
static SenselStatus _senselDecompressionTriggerDetailChange(SENSEL_HANDLE handle)
{
  SenselStatus  status;
  unsigned char data[256];

  status = senselReadRegVS(handle, SENSEL_REG_COMPRESSION_METADATA, sizeof(data), data, NULL);
  if (status != SENSEL_OK)
    return status;

  status = senselDecompressionTriggerDetailChange(handle, data);
  if (status != SENSEL_OK)
    return status;

  return SENSEL_OK;
}
#endif //SENSEL_PRESSURE

SENSEL_API
SenselStatus WINAPI senselSetScanDetail(SENSEL_HANDLE handle, SenselScanDetail detail)
{
  SenselStatus status;

  if (detail >= SCAN_DETAIL_UNKNOWN)
    return SENSEL_ERROR;

  status = senselWriteReg(handle, SENSEL_REG_SCAN_DETAIL_CONTROL, 1, (unsigned char*)&detail);
  if (status != SENSEL_OK)
    return status;
#ifdef SENSEL_PRESSURE
  status = _senselDecompressionTriggerDetailChange(handle);
  if (status != SENSEL_OK)
    return status;
#endif //SENSEL_PRESSURE

  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselGetScanDetail(SENSEL_HANDLE handle, SenselScanDetail *detail)
{
  SenselStatus  status = SENSEL_OK;
  unsigned char tmpdetail;

  status = senselReadReg(handle, SENSEL_REG_SCAN_DETAIL_CONTROL, 1, &tmpdetail);
  if (status != SENSEL_OK)
    return status;

  *detail = tmpdetail;
  return status;
}

static unsigned char _senselParseContactFrame(SenselDevice *device, unsigned char *data_buf, int data_size,
                                     SenselContact *contacts, unsigned char *n_contacts, int *num_bytes_read)
{
  int           num_contacts    = 0;
  int           bit_mask        = 0;
  unsigned char *data_buf_start = data_buf;

  if(data_size < 1)
  {
    printf("Unable to parse number of contacts (data_size == %d)\n", data_size);
    return false;
  }

  *num_bytes_read = 0;
  *n_contacts     = 0;
  bit_mask = *data_buf;
  data_buf++;
  num_contacts = *data_buf;
  data_buf++;
  *num_bytes_read = 2;

  if(num_contacts > 0)
  {
    for (int i = 0; i < num_contacts; i++) {
      contact_raw_t prtcl_raw_contact;

      memcpy(&prtcl_raw_contact, data_buf, CONTACT_DEFAULT_SEND_SIZE);
      data_buf = data_buf + CONTACT_DEFAULT_SEND_SIZE;

      contacts[i].content_bit_mask  = bit_mask;
      contacts[i].id                = (unsigned char)prtcl_raw_contact.id;
      contacts[i].state             = (unsigned int)prtcl_raw_contact.type;
      contacts[i].x_pos             = (float)prtcl_raw_contact.x_pos / device->dims_value_scale;
      contacts[i].y_pos             = (float)prtcl_raw_contact.y_pos / device->dims_value_scale;
      contacts[i].total_force       = (float)prtcl_raw_contact.total_force / device->force_value_scale;
      contacts[i].area              = (float)prtcl_raw_contact.area / device->area_value_scale;

      if (bit_mask & CONTACT_MASK_ELLIPSE)
      {
        memcpy(&prtcl_raw_contact.orientation, data_buf, CONTACT_ELLIPSE_SEND_SIZE);
        data_buf = data_buf + CONTACT_ELLIPSE_SEND_SIZE;

        contacts[i].orientation = (float)prtcl_raw_contact.orientation / device->angle_value_scale;
        contacts[i].major_axis  = (float)prtcl_raw_contact.major_axis / device->dims_value_scale;
        contacts[i].minor_axis  = (float)prtcl_raw_contact.minor_axis / device->dims_value_scale;
      }
      if (bit_mask & CONTACT_MASK_DELTAS)
      {
        memcpy(&prtcl_raw_contact.delta_x, data_buf, CONTACT_DELTAS_SEND_SIZE);
        data_buf = data_buf + CONTACT_DELTAS_SEND_SIZE;

        contacts[i].delta_x     = (float)prtcl_raw_contact.delta_x / device->dims_value_scale;
        contacts[i].delta_y     = (float)prtcl_raw_contact.delta_y / device->dims_value_scale;
        contacts[i].delta_force = (float)prtcl_raw_contact.delta_force / device->force_value_scale;
        contacts[i].delta_area  = (float)prtcl_raw_contact.delta_area / device->area_value_scale;
      }
      if (bit_mask & CONTACT_MASK_BOUNDING_BOX)
      {
        memcpy(&prtcl_raw_contact.min_x, data_buf, CONTACT_BOUNDING_BOX_SEND_SIZE);
        data_buf = data_buf + CONTACT_BOUNDING_BOX_SEND_SIZE;

        contacts[i].min_x = (float)prtcl_raw_contact.min_x / device->dims_value_scale;
        contacts[i].min_y = (float)prtcl_raw_contact.min_y / device->dims_value_scale;
        contacts[i].max_x = (float)prtcl_raw_contact.max_x / device->dims_value_scale;
        contacts[i].max_y = (float)prtcl_raw_contact.max_y / device->dims_value_scale;
      }
      if (bit_mask & CONTACT_MASK_PEAK)
      {
        memcpy(&prtcl_raw_contact.peak_x, data_buf, CONTACT_PEAK_SEND_SIZE);
        data_buf = data_buf + CONTACT_PEAK_SEND_SIZE;

        contacts[i].peak_x      = (float)prtcl_raw_contact.peak_x / device->dims_value_scale;
        contacts[i].peak_y      = (float)prtcl_raw_contact.peak_y / device->dims_value_scale;
        contacts[i].peak_force  = (float)prtcl_raw_contact.peak_force / device->force_value_scale;
      }
    }

    *n_contacts = num_contacts;
    *num_bytes_read = (int)(data_buf - data_buf_start);

#if PRINT_RECEIVED_CONTACTS

    printf("Contact frame size: %d\n", data_size);
    printf("Number of contacts: %d\n", *n_contacts);

    for(int i = 0; i < *n_contacts; i++)
    {
      SenselContact *contact = &(contacts[i]);
      printf("C[%d]:T=%d,F=%f,A=%f,P=(%f,%f),C=(%.2f,%.2f) ||",
          contact->id, (int)contact->state, contact->total_force, contact->area,
          contact->peak_x, contact->peak_y,
          ((float)contact->x_pos)/256.0f, ((float)contact->y_pos)/256.0f);
    }
    printf("\n");
#endif
  }
  return true;
}

static unsigned char _frameBufferEnsureCapacity(SenselDevice *device, int capacity)
{
  // If the read buffer is too small, make it bigger
  if(device->frame_buffer_capacity < capacity)
  {
    // We allocate a buffer twice as big as what we need to avoid
    // very numerous re-allocation. By allocating a buffer
    // twice as big as what is needed, we will be able to accomodate
    // small increases in the data size, and the maximum number of times
    // we'll need to re-allocate is ln2(max_buffer_size)
    device->frame_buffer_capacity = capacity*2;
    device->frame_buffer = (unsigned char*)realloc(device->frame_buffer, device->frame_buffer_capacity);

    if(device->frame_buffer == NULL)
    {
      printf("Unable to allocate temporary buffer!\n");
      return false;
    }
  }

  return true;
}

static unsigned char _frameBufferEnsureAvailability(SenselDevice *device, int additional_capacity)
{
  return _frameBufferEnsureCapacity(device, device->frame_buffer_size + additional_capacity);
}

static unsigned char _senselReadFrameStart(SenselDevice *device)
{
  unsigned char ret;

  read_cmd.reg = SENSEL_REG_SCAN_READ_FRAME;
  read_cmd.size = 0;

  ret = senselSerialWrite(&device->sensor_serial, (unsigned char *)&read_cmd, 3);

  return ret;
}

// This should be a static, but it's referenced externaly when in ASYNC mode
// to clear the communication pipes
unsigned char _senselReadFrame(SenselDevice *device)
{
  int             i;
  unsigned short  payload_size;
  unsigned char   checksum;
  unsigned char   received_checksum;
  unsigned char   reg;
  unsigned char   header;
  unsigned char   *frame_buffer_ptr;

  #if(PRINT_BUFFERING_DEBUG == 1)
    unsigned char content_bit_mask;
    unsigned char rolling_frame_counter;
  #endif
	if (!senselSerialReadBytes(&device->sensor_serial, &reg, 1))
	{
		printf("SENSEL ERROR: Unable to read reg\n");
		return false;
	}
	if (!senselSerialReadBytes(&device->sensor_serial, &header, 1))
	{
		printf("SENSEL ERROR: Unable to read header\n");
		return false;
	}

  if(!senselSerialReadBytes(&device->sensor_serial, (unsigned char *)&payload_size, 2))
  {
    printf("SENSEL ERROR: Unable to read packet size\n");
    return false;
  }

  // Allocate enough space for the size, the data and the checksum
  // Note: This may reallocate the buffer so the pointer to it may change
  if(!_frameBufferEnsureAvailability(device, ((int)payload_size)+3))
  {
    printf("SENSEL ERROR: Unable to allocate buffer\n");
    return false;
  }

  frame_buffer_ptr = &device->frame_buffer[device->frame_buffer_size];

  *((unsigned short*)frame_buffer_ptr) = payload_size;
  frame_buffer_ptr += 2;

  if(!senselSerialReadBytes(&device->sensor_serial, frame_buffer_ptr, payload_size + 1)) //read checksum as well
  {
    printf("SENSEL ERROR: Unable to read frame!\n");
    return false;
  }

  checksum = 0;
  for(i = 0; i < payload_size; i++)
  {
    checksum += *(frame_buffer_ptr++);
  }

  received_checksum = *frame_buffer_ptr;
  if(checksum != received_checksum)
  {
    printf("SENSEL ERROR: Checksum failed! (%d != %d) Dumping the buffer.\n", checksum, received_checksum);
    return false;
  }

  device->frame_buffer_size += payload_size+2; // Grow the buffer by the payload size+2 (we don't count the checksum, so it doesn't end up in the buffer.)
  device->num_buffered_frames++;

  // TODO: I probably shouldn't do this debug stuff here. Instead, I should do it in the code that parses the frame
  #if(PRINT_BUFFERING_DEBUG == 1)
    content_bit_mask = device->frame_buffer[2];
    rolling_frame_counter = device->frame_buffer[3];
    printf("Content bit mask: %d, lost frame count: %d\n", content_bit_mask, rolling_frame_counter);
  #endif

  return true;
}

static unsigned char _senselReadFrames(SenselDevice *device)
{
  unsigned char ack;

  if(device->scan_mode == SCAN_MODE_ASYNC)
  {
    // NOTE: For asynchronous scanning, we peek the ack byte after the first pass.
    // This helps us deal with the situation when there are more than one frames in the queue.
    // NOTE: Right now, I wait for at least one frame to be available. I may want to change it
    // so that this function returns if there are no frames available, or I may want to have an option.
    while(senselSerialGetAvailable(&device->sensor_serial) > 0)
    {
      if(!senselSerialReadBytes(&device->sensor_serial, &ack, 1))
      {
        printf("Failed to receive ack from sensor\n");
        return false;
      }

      if(ack == PT_ASYNC_DATA)
      {
        if(!_senselReadFrame(device))
          return false;
      }
      else
      {
        printf("SENSEL ERROR: Received %d when expecting PT_ASYNC_FRAME.\n", ack);
          return false;
      }
    }
  }
  else if(device->scan_buffer_control == 0)
  {
    if(!senselSerialReadBytes(&device->sensor_serial, &ack, 1))
    {
      printf("Failed to receive ack from sensor\n");
      return false;
    }

    if(ack == PT_RVS_ACK) // Non-buffered frame
    {
      if(!_senselReadFrame(device))
        return false;
    }
    else
    {
      printf("SENSEL ERROR: Received %d when expecting PT_FRAME.\n", ack);
        return false;
    }
  }
  else // scan_buffer_control > 0
  {
    if(!senselSerialReadBytes(&device->sensor_serial, &ack, 1))
    {
      printf("Failed to receive ack from sensor\n");
      return false;
    }

    // One or more buffered frames
    #if(PRINT_BUFFERING_DEBUG == 1)
      printf("Received PT_BUFFERED_FRAME(s)\n");
    #endif

    // Read out buffered frames.
    while(ack == PT_RVS_ACK)
    {
      if(!_senselReadFrame(device))
        return false;

      if(!senselSerialReadBytes(&device->sensor_serial, &ack, 1))
      {
        printf("SENSEL ERROR: Failed to receive ack from sensor\n");
        return false;
      }
    }

    if(ack == PT_BUFFERED_FRAME)
    {
      #if(PRINT_BUFFERING_DEBUG == 1)
        printf("Received end of buffered frames (total of %d frames)\n", device->num_buffered_frames);
      #endif
    }
    else
    {
      printf("SENSEL ERROR: Received %d when expecting PT_BUFFERED_FRAME_END.\n", ack);
      return false;
    }
  }

  return true;
}

SENSEL_API
SenselStatus WINAPI senselReadSensor(SENSEL_HANDLE handle)
{
  SenselDevice *device = (SenselDevice *)handle;

  if(device->scan_mode == SCAN_MODE_SYNC)
  {
    // If we aren't reading asynchronously, we send a start request.
    // Otherwise, we just wait for the data to come in.
    if(!_senselReadFrameStart(device))
    {
      printf("Error: Couldn't initiate the start of a frame read.\n");
      return SENSEL_ERROR;
    }
  }

  if(!_senselReadFrames(device))
  {
    printf("Error reading frame data.\n");
    return SENSEL_ERROR;
  }

  return SENSEL_OK;
}

// Returns number of frames available for reading
SENSEL_API
SenselStatus WINAPI senselGetNumAvailableFrames(SENSEL_HANDLE handle, unsigned int *num_frames)
{
  SenselDevice *device = (SenselDevice *)handle;

  *num_frames = device->num_buffered_frames;
  return SENSEL_OK;
}


static unsigned char _senselParseFrame(SENSEL_HANDLE handle, SenselFrameData *data)
{
  SenselDevice    *device = (SenselDevice*)handle;
  unsigned char   content_bit_mask;
  unsigned char   rolling_frame_counter;
  unsigned short  payload_size;
  unsigned char   *frame_data_ptr;
  unsigned int    timestamp;
  int             frame_size;
  int             frame_data_size = 0;

  //////////////////////////////////////
  // Extract the payload size
  if(device->frame_buffer_size < 2)
  {
    printf("Error: Payload size not found in buffer.\n");
    return false;
  }

  payload_size = *((unsigned short*)device->frame_buffer);
  frame_size = payload_size + 2; // Size of frame (including the two payload size bytes)

  frame_data_ptr = device->frame_buffer + 2; // Skip the payload size
  frame_data_size = payload_size;

  //////////////////////////////////////
  // Extract the content bit mask and lost frame count
  if(frame_data_size < 2)
  {
    printf("Error: Frame doesn't have content bit mask and/or lost frame count.\n");
    return false;
  }

  content_bit_mask = frame_data_ptr[0];
  rolling_frame_counter = frame_data_ptr[1];
  //printf("%d\n", rolling_frame_counter);

  memcpy((unsigned char *)&timestamp, (unsigned char *)&(frame_data_ptr[2]), 4);
  //printf("Time: %8d  /\\ = %d\n", timestamp, timestamp -prev_timestamp);
  device->prev_timestamp = timestamp;

  frame_data_ptr += 6;
  frame_data_size -= 6;

  // Fill in frame_info
  int elapsed_frames = 0;
  elapsed_frames = (int)rolling_frame_counter - (int)device->prev_rolling_frame_counter;
  if(elapsed_frames <= 0) elapsed_frames += 256;

  data->content_bit_mask = content_bit_mask;
  data->lost_frame_count = (unsigned int)(elapsed_frames - 1);

  device->prev_rolling_frame_counter = rolling_frame_counter;

	//////////////////////////////////////
	// Extract the contacts if available
	if (content_bit_mask & FRAME_CONTENT_CONTACTS_MASK)
	{
		int num_decompressed_bytes = 0;

		//printf("packet size: %d, num_bytes_read: %d\n", payload_size, num_decompressed_bytes);

		if (!_senselParseContactFrame(device, frame_data_ptr, frame_data_size, data->contacts, &data->n_contacts, &num_decompressed_bytes))
		{
			printf("Error while decompressing contacts!\n");
			return false;
		}
		frame_data_ptr += num_decompressed_bytes;
		frame_data_size -= num_decompressed_bytes;
	}
	else
	{
		data->n_contacts = 0;
	}

	///////////////////////////////////////////
	// Extract accelerometer data if available
	if (content_bit_mask & FRAME_CONTENT_ACCEL_MASK)
	{
    sensel_accel_data_t prv_accel_data;

		memcpy(&(prv_accel_data), frame_data_ptr, sizeof(sensel_accel_data_t));
    frame_data_ptr  += sizeof(sensel_accel_data_t);
		frame_data_size -= sizeof(sensel_accel_data_t);

    data->accel_data->x = prv_accel_data.x;
    data->accel_data->y = prv_accel_data.y;
    data->accel_data->z = prv_accel_data.z;
	}

#ifdef SENSEL_PRESSURE
  //////////////////////////////////////
  // Extract the pressure map if available

  if (content_bit_mask & FRAME_CONTENT_PRESSURE_MASK || content_bit_mask & FRAME_CONTENT_LABELS_MASK)
  {
    unsigned int    decompress_bytes_read;

    if(senselDecompressFrame(handle, frame_data_ptr, frame_data_size, content_bit_mask, data, &decompress_bytes_read))
    {
      printf("Error while decompressiong data\n");
      return false;

    }
    frame_data_ptr  += decompress_bytes_read;
    frame_data_size -= decompress_bytes_read;
  }
#else
  frame_data_ptr  += frame_data_size;
  frame_data_size  = 0;
#endif // SENSEL_PRESSURE

  //////////////////////////////////////
  // Verify that the frame size was correct
  if(frame_data_size != 0)
  {
    printf("Error while decompressing frame. %d unread bytes left in buffer.\n", frame_data_size);
    return false;
  }

  //////////////////////////////////////
  // Remove the frame from the frame buffer
  // NOTE: I use memmove instead of memcopy because memcopy results in data corruption when the two arrays overlap
  //       I should consider just using a for-loop and copying one byte at a time for safety.
  memmove(device->frame_buffer, device->frame_buffer+frame_size, device->frame_buffer_size-frame_size);
  device->frame_buffer_size -= frame_size;

  return true;
}

SENSEL_API
SenselStatus WINAPI senselGetFrame(SENSEL_HANDLE handle, SenselFrameData *data)
{
  SenselDevice *device = (SenselDevice*)handle;

  if(device->num_buffered_frames <= 0)
  {
    return SENSEL_ERROR;
    printf("Error: No frames available.\n");
  }

  if(!_senselParseFrame(handle, data))
  {
    // TODO: Properly handle the case where we can't properly parse a frame.
    //       We should probably delete that frame. We might want to keep frames
    //       in separate buffers to make the error handling more robust.
    return SENSEL_ERROR;
  }

  device->num_buffered_frames --;

  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselSetBufferControl(SENSEL_HANDLE handle, unsigned char num)
{
  SenselDevice *device = (SenselDevice *)handle;
  SenselStatus status  = SENSEL_OK;

  if(!device)
    return SENSEL_ERROR;

  status = senselWriteReg(handle, SENSEL_REG_SCAN_BUFFER_CONTROL, 1, &num);
  if(status != SENSEL_OK)
  {
    printf("Error: Unable to set num_buffers.\n");
    return status;
  }

  device->scan_buffer_control = num;

  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselGetBufferControl(SENSEL_HANDLE handle, unsigned char *num)
{
  SenselDevice *device = (SenselDevice *)handle;

  return senselReadReg(device, SENSEL_REG_SCAN_BUFFER_CONTROL, 1, num);
}

SENSEL_API
SenselStatus WINAPI senselSetScanMode(SENSEL_HANDLE handle, SenselScanMode mode)
{
  SenselDevice  *device = (SenselDevice *)handle;
  SenselStatus  status  = SENSEL_OK;
  unsigned char val;

  if (mode == SCAN_MODE_SYNC)
  {
    val = 1;
    if (device->scanning_active == true)
      status = senselWriteReg(handle, SENSEL_REG_SCAN_ENABLED, 1, &val);
    if (status != SENSEL_OK)
      return status;
    device->scan_mode = SCAN_MODE_SYNC;
  }
  else if (mode == SCAN_MODE_ASYNC)
  {
    val = 2;
    if (device->scanning_active == true)
      status = senselWriteReg(handle, SENSEL_REG_SCAN_ENABLED, 1, &val);
    if (status != SENSEL_OK)
      return status;
    device->scan_mode = SCAN_MODE_ASYNC;
  }
  else
  {
    return SENSEL_ERROR;
  }
  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselGetScanMode(SENSEL_HANDLE handle, SenselScanMode *mode)
{
  SenselDevice *device = (SenselDevice *)handle;

  *mode = device->scan_mode;
  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselStartScanning(SENSEL_HANDLE handle)
{
  SenselDevice  *device = (SenselDevice*)handle;
  SenselStatus  status;
  unsigned char val;

  if (device->scanning_active == true)
    return SENSEL_OK;

  device->num_buffered_frames = 0;
  device->frame_buffer_size = 0;
  device->prev_rolling_frame_counter = 255;

  if(device->scan_mode == SCAN_MODE_SYNC)
  {
    val = SCAN_MODE_SYNC;
    status = senselWriteReg(handle, SENSEL_REG_SCAN_ENABLED, 1, &val);
  }
  else if(device->scan_mode == SCAN_MODE_ASYNC)
  {
    val = SCAN_MODE_ASYNC;
    status = senselWriteReg(handle, SENSEL_REG_SCAN_ENABLED, 1, &val);
  }
  else
  {
    return SENSEL_ERROR;
  }

  if (status != SENSEL_OK)
    return status;

  device->scanning_active = true;

  return SENSEL_OK;
}

// TODO: We have no way to properly handle stopping of asynchronous scanning,
// because we may still have stuff in the queue when we stop.
SENSEL_API
SenselStatus WINAPI senselStopScanning(SENSEL_HANDLE handle)
{
  SenselDevice  *device = (SenselDevice*)handle;
  unsigned char val     = SCAN_MODE_DISABLE;

  device->scanning_active   = false;

  return senselWriteReg(handle, SENSEL_REG_SCAN_ENABLED, 1, &val);
}

SENSEL_API
SenselStatus WINAPI senselGetSupportedFrameContent(SENSEL_HANDLE handle, unsigned char *content)
{
  return senselReadReg(handle, SENSEL_REG_FRAME_CONTENT_SUPPORTED, 1, content);
}

SENSEL_API
SenselStatus WINAPI senselSetFrameContent(SENSEL_HANDLE handle, unsigned char content)
{
  SenselDevice *device = (SenselDevice*)handle;

  // Make sure that all features requested are supported by the device
  if (content != (content & device->supported_frame_content))
    return SENSEL_ERROR;

  // Make sure to disable the pressure mask reporting if not enable
  #ifndef SENSEL_PRESSURE
  if (content & FRAME_CONTENT_PRESSURE_MASK)
    content &= ~FRAME_CONTENT_PRESSURE_MASK;
  if (content & FRAME_CONTENT_LABELS_MASK)
    content &= ~FRAME_CONTENT_LABELS_MASK;
  #endif //SENSEL_PRESSURE

  if(content == device->frame_content_control)
  {
    return SENSEL_OK;
  }
  else
  {
    device->frame_content_control = content;
    return senselWriteReg(handle, SENSEL_REG_FRAME_CONTENT_CONTROL, 1, &device->frame_content_control);
  }
}

SENSEL_API
SenselStatus WINAPI senselGetFrameContent(SENSEL_HANDLE handle, unsigned char *content)
{
  return senselReadReg(handle, SENSEL_REG_FRAME_CONTENT_CONTROL, 1, content);
}

SENSEL_API
SenselStatus WINAPI senselSetMaxFrameRate(SENSEL_HANDLE handle, unsigned short val)
{
  return senselWriteReg(handle, SENSEL_REG_SCAN_FRAME_RATE, 2, (unsigned char*)&val);
}

SENSEL_API
SenselStatus WINAPI senselGetMaxFrameRate(SENSEL_HANDLE handle, unsigned short *val)
{
  return senselReadReg(handle, SENSEL_REG_SCAN_FRAME_RATE, 2, (unsigned char*)val);
}

SENSEL_API
SenselStatus WINAPI senselSetContactsEnableBlobMerge(SENSEL_HANDLE handle, unsigned char val)
{
  return senselWriteReg(handle, SENSEL_REG_CONTACTS_ENABLE_BLOB_MERGE, 1, &val);
}

SENSEL_API
SenselStatus WINAPI senselGetContactsEnableBlobMerge(SENSEL_HANDLE handle, unsigned char *val)
{
  return senselReadReg(handle, SENSEL_REG_CONTACTS_ENABLE_BLOB_MERGE, 1, val);
}

SENSEL_API
SenselStatus WINAPI senselSetContactsMinForce(SENSEL_HANDLE handle, unsigned short val)
{
  return senselWriteReg(handle, SENSEL_REG_CONTACTS_MIN_FORCE, 2, (unsigned char*)&val);
}

SENSEL_API
SenselStatus WINAPI senselGetContactsMinForce(SENSEL_HANDLE handle, unsigned short *val)
{
  return senselReadReg(handle, SENSEL_REG_CONTACTS_MIN_FORCE, 2, (unsigned char*)val);
}

SENSEL_API
SenselStatus WINAPI senselSetContactsMask(SENSEL_HANDLE handle, unsigned char mask)
{
  return senselWriteReg(handle, SENSEL_REG_CONTACTS_MASK, 1, &mask);
}

SENSEL_API
SenselStatus WINAPI senselGetContactsMask(SENSEL_HANDLE handle, unsigned char *mask)
{
  return senselReadReg(handle, SENSEL_REG_CONTACTS_MASK, 1, mask);
}

SENSEL_API
SenselStatus WINAPI senselSetDynamicBaselineEnabled(SENSEL_HANDLE handle, unsigned char val)
{
  SenselStatus  status;
  SenselDevice  *device = (SenselDevice *)handle;

  val = (val ? 1 : 0);
  if (val == device->dynamic_baseline_enabled)
    return SENSEL_OK;


  status = senselWriteReg(handle, SENSEL_REG_BASELINE_DYNAMIC_ENABLED, SENSEL_REG_SIZE_BASELINE_DYNAMIC_ENABLED, &val);
  if (status != SENSEL_OK)
    return status;

  device->dynamic_baseline_enabled = val;
  return SENSEL_OK;
}

static
SenselStatus _senselGetDynamicBaselineEnabled(SENSEL_HANDLE handle, unsigned char *val)
{
  return senselReadReg(handle, SENSEL_REG_BASELINE_DYNAMIC_ENABLED, SENSEL_REG_SIZE_BASELINE_DYNAMIC_ENABLED, val);
}

SENSEL_API
SenselStatus WINAPI senselGetDynamicBaselineEnabled(SENSEL_HANDLE handle, unsigned char *val)
{
  SenselDevice *device = (SenselDevice *)handle;

  *val = device->dynamic_baseline_enabled;
  return SENSEL_OK;
}

#ifdef SENSEL_PRESSURE
static SenselStatus _senselInitDecompressionHandle(SENSEL_HANDLE handle)
{
  SenselStatus  status;
  unsigned char data[256];

  status = senselReadRegVS(handle, SENSEL_REG_COMPRESSION_METADATA, sizeof(data), data, NULL);
  if (status != SENSEL_OK)
    return status;

  return senselInitDecompressionHandle(handle, data);
}
#endif //SENSEL_PRESSURE

static SenselStatus _senselInitHandle(SENSEL_HANDLE handle)
{
  SenselDevice *device = (SenselDevice*) handle;
  SenselStatus status  = SENSEL_OK;

  device->supported_frame_content    = 0;
  device->frame_content_control      = DEFAULT_FRAME_CONTENT_CONTROL;
  device->scan_buffer_control        = 0;
  device->scan_mode                  = SCAN_MODE_SYNC;
  device->num_buffered_frames        = 0;
  device->prev_rolling_frame_counter = 0;
  device->prev_timestamp             = 0;
  device->dynamic_baseline_enabled   = 1;
  device->scanning_active            = false;
  device->decomp_handle              = NULL;
  device->num_leds                   = 0;
  device->max_led_brightness         = 0;
  device->led_reg_size               = 1;

  status = _senselGetPrvFirmwareInfo(handle, &device->fw_info);
  if(status != SENSEL_OK)
    return status;

  status = senselGetFrameContent(handle, &device->frame_content_control);
  if (status != SENSEL_OK)
    return status;

  status = senselGetBufferControl(handle, &device->scan_buffer_control);
  if (status != SENSEL_OK)
    return status;

  status = senselGetSupportedFrameContent(handle, &device->supported_frame_content);
  if (status != SENSEL_OK)
    return status;

  status = _senselGetDynamicBaselineEnabled(handle, &device->dynamic_baseline_enabled);
  if (status != SENSEL_OK)
    return status;

  // Initialize SenselSensorInfo structure
  status = _senselGetSensorMaxContacts(handle, &device->sensor_info.max_contacts);
  if (status != SENSEL_OK)
    return status;

  status = _senselGetSensorNumRows(handle, &device->sensor_info.num_rows);
  if(status != SENSEL_OK)
    return status;

  status = _senselGetSensorNumCols(handle, &device->sensor_info.num_cols);
  if(status != SENSEL_OK)
    return status;

  status = _senselGetDimsValueScale(handle, &device->dims_value_scale);
  if (status != SENSEL_OK)
    return status;

  status = _senselGetForceValueScale(handle, &device->force_value_scale);
  if (status != SENSEL_OK)
    return status;

  status = _senselGetAngleValueScale(handle, &device->angle_value_scale);
  if (status != SENSEL_OK)
    return status;

  status = _senselGetAreaValueScale(handle, &device->area_value_scale);
  if (status != SENSEL_OK)
    return status;

  unsigned int width;
  status = _senselGetSensorWidthUM(handle, &width);
  if (status != SENSEL_OK)
    return status;
  device->sensor_info.width = (float)width / 1000.0f;

  unsigned int height;
  status = _senselGetSensorHeightUM(handle, &height);
  if (status != SENSEL_OK)
    return status;
  device->sensor_info.height = (float)height / 1000.0f;

#ifdef SENSEL_PRESSURE
  status = _senselInitDecompressionHandle(handle);
  if(status != SENSEL_OK)
    return status;
#endif //SENSEL_PRESSURE

  device->frame_buffer = (unsigned char*)malloc(FRAME_BUFFER_INITIAL_CAPACITY*sizeof(unsigned char));
  if(!device->frame_buffer)
  {
    printf("Error allocating frame buffer.\n");
    return SENSEL_ERROR;
  }
  device->frame_buffer_capacity = FRAME_BUFFER_INITIAL_CAPACITY;
  device->frame_buffer_size = 0;

  // Init LED related settings
  status = senselGetNumAvailableLEDs(handle, &device->num_leds);
  if (status != SENSEL_OK)
    return status;

  status = senselGetMaxLEDBrightness(handle, &device->max_led_brightness);
  if (status != SENSEL_OK)
    return status;

  status = _senselGetLEDRegSize(handle, &device->led_reg_size);
  if (status != SENSEL_OK)
    return status;

  if (device->num_leds)
  {
    device->led_array = malloc(device->num_leds * device->led_reg_size * sizeof(unsigned char));
    if (!device->led_array)
    {
      printf("Error allocating memory for LED array\n");
    }
    status = _senselGetAllLEDBrightness(handle);
    if (status != SENSEL_OK)
      return status;
  }
  else
  {
    device->led_array = NULL;
  }

  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselGetDeviceList(SenselDeviceList *list)
{
  if (!list)
    return SENSEL_ERROR;

  senselSerialScan(list);
  return SENSEL_OK;
}

SENSEL_API
SenselStatus WINAPI senselSoftReset(SENSEL_HANDLE handle)
{
  SenselStatus  status  = SENSEL_OK;
  SenselDevice  *device = (SenselDevice*) handle;
  unsigned char val     = 1;

  if (!device)
    return SENSEL_ERROR;

  status = senselWriteReg(handle, SENSEL_REG_SOFT_RESET, 1, &val);
  if (status != SENSEL_OK)
    return status;

  /*
   * Any memory allocated in _senselInitHandle must be cleared here to ensure
   * no memory leak.
   */
  CHECK_FREE(device->frame_buffer);
  CHECK_FREE(device->led_array);

#ifdef SENSEL_PRESSURE
  if (device->decomp_handle)
    senselFreeDecompressionHandle(handle);
#endif //SENSEL_PRESSURE

  return _senselInitHandle(handle);
}

SENSEL_API
SenselStatus WINAPI senselOpenDeviceByID(SENSEL_HANDLE *handle, unsigned char idx)
{
  SenselStatus status  = SENSEL_OK;
  SenselDevice *device = malloc(sizeof(SenselDevice));

  if (!device)
    return SENSEL_ERROR;

  memset(device, 0, sizeof(SenselDevice));
  *handle = device;

  if (!senselSerialOpenDeviceByID(&device->sensor_serial, idx))
    goto error;

  status = senselSoftReset(*handle);
  if (status != SENSEL_OK)
  {
    printf("Error resetting sensor settings.\n");
    goto error;
  }

  return SENSEL_OK;

error:
  free(device);
  return SENSEL_ERROR;
}

SENSEL_API
SenselStatus WINAPI senselOpenDeviceBySerialNum(SENSEL_HANDLE *handle, unsigned char *serial_num)
{
  SenselStatus status   = SENSEL_OK;
  SenselDevice *device  = malloc(sizeof(SenselDevice));

  if (!device)
    return SENSEL_ERROR;

  memset(device, 0, sizeof(SenselDevice));
  *handle = device;

  if (!senselSerialOpenDeviceBySerialNum(&device->sensor_serial, (char*)serial_num))
    goto error;

  status = senselSoftReset(*handle);
  if (status != SENSEL_OK)
  {
    printf("Error resetting sensor settings.\n");
    goto error;
  }

  return SENSEL_OK;

error:
  free(device);
  return SENSEL_ERROR;
}

SENSEL_API
SenselStatus WINAPI senselOpenDeviceByComPort(SENSEL_HANDLE *handle, unsigned char *com_port)
{
  SenselStatus status   = SENSEL_OK;
  SenselDevice *device  = malloc(sizeof(SenselDevice));

  if (!device)
    return SENSEL_ERROR;

  memset(device, 0, sizeof(SenselDevice));
  *handle = device;

  if (!senselSerialOpenDeviceByComPort(&device->sensor_serial, (char*)com_port))
    goto error;

  status = senselSoftReset(*handle);
  if (status != SENSEL_OK)
  {
    printf("Error resetting sensor settings.\n");
    goto error;
  }

  return SENSEL_OK;

error:
  free(device);
  return SENSEL_ERROR;
}

SENSEL_API
SenselStatus WINAPI senselOpen(SENSEL_HANDLE *handle)
{
  SenselStatus status   = SENSEL_OK;
  SenselDevice *device  = malloc(sizeof(SenselDevice));

  if (!device)
    return SENSEL_ERROR;

  memset(device, 0, sizeof(SenselDevice));
  *handle = device;

  // Connect to the sensor
  if(!senselSerialOpen(&device->sensor_serial, NULL))
    goto error;

  status = senselSoftReset(*handle);
  if (status != SENSEL_OK)
  {
    printf("Error resetting sensor settings.\n");
    goto error;
  }

  return SENSEL_OK;
error:
  printf("Error\n");
  free(device);
  return SENSEL_ERROR;
}

SENSEL_API
SenselStatus WINAPI senselClose(SENSEL_HANDLE handle)
{
  SenselDevice *device = (SenselDevice*)handle;

  senselSoftReset(handle);
  senselSerialClose(&device->sensor_serial);

  CHECK_FREE(device->frame_buffer);
  CHECK_FREE(device->led_array);

  #ifdef SENSEL_PRESSURE
    if (device->decomp_handle)
      senselFreeDecompressionHandle(handle);
  #endif

  free(device);
  return SENSEL_OK;
}
