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

#ifndef __SENSEL_H__
#define __SENSEL_H__

#ifdef WIN32
  #include <windows.h>
#else
  #define WINAPI
#endif

#ifdef SENSEL_EXPORTS
  #define SENSEL_API __declspec(dllexport)
#else
  #define SENSEL_API __declspec(dllimport)
#endif

#ifndef WIN32
  #ifdef SENSEL_API
    #undef SENSEL_API
    #define SENSEL_API __attribute__ ((visibility ("default")))
  #endif
#endif

#define SENSEL_MAX_DEVICES             16    // Maximum number of devices supported by the API

#define FRAME_CONTENT_PRESSURE_MASK    0x01  // Mask indicating that the frame includes pressure data
#define FRAME_CONTENT_LABELS_MASK      0x02  // Mask indicating that the frame includes labels data
#define FRAME_CONTENT_CONTACTS_MASK    0x04  // Mask indicating that the frame includes contacts data
#define FRAME_CONTENT_ACCEL_MASK       0x08  // Mask indicating that the frame includes acceleromter data

#define CONTACT_MASK_ELLIPSE           0x01  // Mask indicating that the contact data contains ellipse info
#define CONTACT_MASK_DELTAS            0x02  // Mask indicating that the contact data contains deltas info
#define CONTACT_MASK_BOUNDING_BOX      0x04  // Mask indicating that the contact data contains bound box info
#define CONTACT_MASK_PEAK              0x08  // Mask indicating that the contact data contains peak info

#ifdef __cplusplus
extern "C" {
#endif

  /*
   * Name       : SENSEL_HANDLE
   * Description: Handle to a Sensel device
   */
  typedef void *SENSEL_HANDLE;

  /*
   * Name       : SenselStatus
   * Description: Status returned by API calls
   */
  typedef enum
  {
    SENSEL_OK        =  0,              // Call was successful
    SENSEL_ERROR     = -1,              // Call returned an error
  } SenselStatus;

	/*
	 * Name				: SenselScanMode
	 * Description: Device scan reporting mode
	 */
  typedef enum
  {
    SCAN_MODE_DISABLE,
    SCAN_MODE_SYNC,
    SCAN_MODE_ASYNC,
  } SenselScanMode;

  /*
   * Name       : SenselScanDetail
   * Description: Describes the current state of a contact
   */
  typedef enum
  {
    SCAN_DETAIL_HIGH     = 0,           // Scan at full resolution
    SCAN_DETAIL_MEDIUM   = 1,           // Scan at half resolution
    SCAN_DETAIL_LOW      = 2,           // Scan at quarter resolution
    SCAN_DETAIL_UNKNOWN  = 3,
  } SenselScanDetail;

  /*
   * Name       : SenselContactState
   * Description: Describes the current state of a contact
   */
  typedef enum
  {
    CONTACT_INVALID = 0,                // Contact is invalid
    CONTACT_START   = 1,                // Contact has started
    CONTACT_MOVE    = 2,                // Contact has moved
    CONTACT_END     = 3,                // Contact has ended
  } SenselContactState;

  /*
   * Name       : SenselSensorInfo
   * Description: Structure containing sensor information
   */
  typedef struct
  {
    unsigned char   max_contacts;       // Maximum number of contacts the sensor supports
    unsigned short  num_rows;           // Total number of rows
    unsigned short  num_cols;           // Total number of columns
    float           width;              // Width of the sensor in millimeters
    float           height;             // Height of the sensor in millimeters
  } SenselSensorInfo;

  /*
   * Name       : SenselFirmwareInfo
   * Description: Structure containing firmware information
   */
  typedef struct
  {
    unsigned char  fw_protocol_version; // Sensel communication protocol supported by the device
    unsigned char  fw_version_major;    // Major version of the firmware
    unsigned char  fw_version_minor;    // Minor version of the firmware
    unsigned short fw_version_build;    // ??
    unsigned char  fw_version_release;  // ??
    unsigned short device_id;           // Sensel device type
    unsigned char  device_revision;     // Device revision
  } SenselFirmwareInfo;


  /*
   * Name       : SenselContact
   * Description: Structure containing all information related to a detected contact
   *              The content_bit_mask reflects which data is valid
   */
	typedef struct
	{
    unsigned char        content_bit_mask;   // Mask of what contact data is valid
    unsigned char        id;                 // Contact id
    unsigned int         state;              // Contact state (enum SenselContactState)
    float                x_pos;              // X position in mm
    float                y_pos;              // Y position in mm
    float                total_force;        // Total contact force in grams
    float                area;               // Area in sensor elements

    // CONTACT_MASK_ELLIPSE
    float                orientation;        // Angle in degrees
    float                major_axis;         // Length of the major axis in mm
    float                minor_axis;         // Length of the minor axis in mm

    // CONTACT_MASK_DELTAS
    float                delta_x;            // X contact displacement in mm
    float                delta_y;            // Y contact displacement in mm
    float                delta_force;        // Force delta in grams
    float                delta_area;         // Area delta in sensor elements

    // CONTACT_MASK_BOUNDING_BOX
    float                min_x;              // Bounding box min X coordinate in mm
    float                min_y;              // Bounding box min Y coordinate in mm
    float                max_x;              // Bounding box max X coordinate in mm
    float                max_y;              // Bounding box max Y coordinate in mm

    // CONTACT_MASK_PEAK
    float                peak_x;             // X position of the peak in mm
    float                peak_y;             // Y position of the peak in mm
    float                peak_force;         // Peak force in grams
  } SenselContact;

  /*
   * Name       : SenselAccelData
   * Description: Accelerometer information
   */
  typedef struct
  {
    int x;                             // X axis acceleration
    int y;                             // Y axis acceleration
    int z;                             // Z axis acceleration
  } SenselAccelData;

  /*
   * Name       : SenselFrameData
   * Description: Container for one frame of data.
   *              Available content of the frame needs to be checked by looking at the content_bit_mask
   */
  typedef struct
  {
    unsigned char   content_bit_mask;  // Data contents of the frame
    int             lost_frame_count;  // Number of frames dropped
    unsigned char   n_contacts;        // Number of contacts
    SenselContact   *contacts;         // Array of contacts
    float           *force_array;      // Force image buffer
    unsigned char   *labels_array;     // Labels buffer
    SenselAccelData *accel_data;       // Accelerometer data
  } SenselFrameData;

  /*
   * Name       : SenselDeviceID
   * Description: Sensel identifier information
   */
  typedef struct
  {
    unsigned char idx;                 // ID of the sensor
    unsigned char serial_num[64];      // Serial number of the sensor
    unsigned char com_port[64];        // Com port associated with the sensor
  } SenselDeviceID;

  /*
   * Name       : SenselDeviceList
   * Description: List of connected Sensel devices
   */
  typedef struct
  {
    unsigned char  num_devices;                 // Num of devices found
    SenselDeviceID devices[SENSEL_MAX_DEVICES]; // Sensel device ID details
  } SenselDeviceList;

  /*
   * Function   : senselOpen
   * Parameters : handle: Sensel device handle to be allocated
   * Return     : SENSEL_OK on success or error
   * Description: Opens the first available Sensel sensor
   */
  SENSEL_API
  SenselStatus WINAPI senselOpen(SENSEL_HANDLE *handle);

  /*
   * Function   : senselGetDeviceList
   * Parameters : list: Device list to be populated
   * Return     : SENSEL_OK on success or error
   * Description: Scans for all availale sensel devices and populates list accordingly
   */
  SENSEL_API
  SenselStatus WINAPI senselGetDeviceList(SenselDeviceList *list);

  /*
   * Function   : senselOpenDeviceByID
   * Parameters : handle: Sensel device handle to be initialized
   *              idx   : identifier of the device to open
   * Return     : SENSEL_OK on success or error
   * Description: Opens the devices associated to the given idx as returned by senselGetDeviceList.
   *              senselGetDeviceList must be called prior to this call
   */
  SENSEL_API
  SenselStatus WINAPI senselOpenDeviceByID(SENSEL_HANDLE *handle, unsigned char idx);

  /*
   * Function   : senselOpenDeviceBySerialNum
   * Parameters : handle    : Sensel device handle to be initialized
   *              serial_num: serial_number of the device to open
   * Return     : SENSEL_OK on success or error
   * Description: Opens the devices associated to the given serial_num as returned by senselGetDeviceList.
   *              senselGetDeviceList must be called prior to this call
   */
  SENSEL_API
  SenselStatus WINAPI senselOpenDeviceBySerialNum(SENSEL_HANDLE *handle, unsigned char *serial_num);

  /*
   * Function   : senselOpenDeviceByComPort
   * Parameters : handle  : Sensel device handle to be initialized
   *              com_port: com_port path of the device to open
   * Return     : SENSEL_OK on success or error
   * Description: Opens the devices associated to the given com_port as returned by senselGetDeviceList.
   *              senselGetDeviceList must be called prior to this call
   */
  SENSEL_API
  SenselStatus WINAPI senselOpenDeviceByComPort(SENSEL_HANDLE *handle, unsigned char *com_port);

  /*
   * Function   : senselClose
   * Parameters : handle: Sensel device to be closed
   * Return     : SENSEL_OK on success or error
   * Description: Closes the device associated with handle and frees all memory.
   */
  SENSEL_API
  SenselStatus WINAPI senselClose(SENSEL_HANDLE handle);

  /*
   * Function   : senselSoftReset
   * Parameters : handle: Sensel device to reset
   * Return     : SENSEL_OK on success or error
   * Description: Executes a soft reset the device referenced by handle.
   *              All registers are reset to their initial state
   */
  SENSEL_API
  SenselStatus WINAPI senselSoftReset(SENSEL_HANDLE handle);

  /*
   * Function   : senselGetSensorInfo
   * Parameters : handle: Sensel device handle to information about
   *              info  : Pointer to a structure to populate
   * Return     : SENSEL_OK on success or error
   * Description: Retrieves the sensor information
   */
  SENSEL_API
  SenselStatus WINAPI senselGetSensorInfo(SENSEL_HANDLE handle, SenselSensorInfo *info);

  /*
   * Function   : senselAllocateFrameData
   * Parameters : handle: Sensel device handle for which to create a FrameData structure for
   *              data  : Pointer to FrameData to allocate.
   * Return     : SENSEL_OK on success or error
   * Description: Allocates a FrameData and initializes all buffers according to device capabilities.
   */
  SENSEL_API
  SenselStatus WINAPI senselAllocateFrameData(SENSEL_HANDLE handle, SenselFrameData **data);

  /*
   * Function   : senselFreeFrameData
   * Parameters : handle: Sensel device handle
   *              data  : FrameData to free.
   * Return     : SENSEL_OK on success or error
   * Description: Frees all memory allocated to FrameData including the FrameData itself.
   */
  SENSEL_API
  SenselStatus WINAPI senselFreeFrameData(SENSEL_HANDLE handle, SenselFrameData *data);

  /*
   * Function   : senselSetScanDetail
   * Parameters : handle: Sensel device handle
   *              detail: Scan detail level
   * Return     : SENSEL_OK on success or error
   * Description: Set the level of scanning detail returned by the device. This setting trades precision for performance.
   */
  SENSEL_API
  SenselStatus WINAPI senselSetScanDetail(SENSEL_HANDLE handle, SenselScanDetail detail);

  /*
   * Function   : senselGetScanDetail
   * Parameters : handle: Sensel device handle
   *              detail: Pointer to scan detail level to retrieve
   * Return     : SENSEL_OK on success or error
   * Description: Get the current scanning level setting
   */
  SENSEL_API
  SenselStatus WINAPI senselGetScanDetail(SENSEL_HANDLE handle, SenselScanDetail *detail);

  /*
   * Function   : senselGetSupportedFrameContent
   * Parameters : handle  : Sensel device handle
   *              content : Pointer to frame content supported by device
   * Return     : SENSEL_OK on success or error
   * Description: Retrieve the frame content that the device supports
   */
  SENSEL_API
  SenselStatus WINAPI senselGetSupportedFrameContent(SENSEL_HANDLE handle, unsigned char *content);

  /*
   * Function   : senselSetFrameContent
   * Parameters : handle  : Sensel device handle
   *              content : Frame content mask
   * Return     : SENSEL_OK on success or error
   * Description: Sets the information returned by the sensor.
   *              content can be any combination of FRAME_CONTENT_*_MASK.
   *              FrameData returned in subsequent GetFrame calls will reflect this setting.
   */
  SENSEL_API
  SenselStatus WINAPI senselSetFrameContent(SENSEL_HANDLE handle, unsigned char content);

  /*
   * Function   : senselGetFrameContent
   * Parameters : handle  : Sensel device handle
   *              content : Pointer to content level to retrieve
   * Return     : SENSEL_OK on success or error
   * Description: Get the current frame content mask from the device
   */
  SENSEL_API
  SenselStatus WINAPI senselGetFrameContent(SENSEL_HANDLE handle, unsigned char *content);

  /*
   * Function   : senselStartScanning
   * Parameters : handle: Sensel device handle
   * Return     : SENSEL_OK on success or error
   * Description: Start sensor scanning
   */
  SENSEL_API
  SenselStatus WINAPI senselStartScanning(SENSEL_HANDLE handle);

  /*
   * Function   : senselStopScanning
   * Parameters : handle: Sensel device handle
   * Return     : SENSEL_OK on success or error
   * Description: Stop sensor scanning
   */
  SENSEL_API
  SenselStatus WINAPI senselStopScanning(SENSEL_HANDLE handle);

  /*
   * Function   : senselReadSensor
   * Parameters : handle: Sensel device handle
   * Return     : SENSEL_OK on success or error
   * Description: Reads out and stores all pending frames from the sensor
   */
  SENSEL_API
  SenselStatus WINAPI senselReadSensor(SENSEL_HANDLE handle);

  /*
   * Function   : senselGetNumAvailableFrames
   * Parameters : handle          : Sensel device handle
   *              num_avail_frames: Will contain the number of frames available to GetFrame
   * Return     : SENSEL_OK on success or error
   * Description: Returns in num_avail_frames the number of data frames available.
   */
  SENSEL_API
  SenselStatus WINAPI senselGetNumAvailableFrames(SENSEL_HANDLE handle, unsigned int *num_avail_frames);

  /*
   * Function   : senselGetFrame
   * Parameters : handle: Sensel device handle
   *              data  : Pointer to pre-allocated FrameData to populate
   * Return     : SENSEL_OK on success or error
   * Description: Returns one frame of data in data.
   */
  SENSEL_API
  SenselStatus WINAPI senselGetFrame(SENSEL_HANDLE handle, SenselFrameData *data);

  /*
   * Function   : senselSetDynamicBaselineEnabled
   * Parameters : handle: Sensel device handle
   *              val   : Enable/Disable flag
   * Return     : SENSEL_OK on success or error
   * Description: Enables or disables dynamic baselining based on val. If dynamic baseline is
   *              disabled, the baseline does not evolve over time.
   */
  SENSEL_API
  SenselStatus WINAPI senselSetDynamicBaselineEnabled(SENSEL_HANDLE handle, unsigned char val);

  /*
   * Function   : senselGetDynamicBaselineEnabled
   * Parameters : handle: Sensel device handle
   *              val   : Pointer to enable/disable flag
   * Return     : SENSEL_OK on success or error
   * Description: Returns state of dynamic baseline enabled flag
   */
  SENSEL_API
  SenselStatus WINAPI senselGetDynamicBaselineEnabled(SENSEL_HANDLE handle, unsigned char *val);

  /*
   * Function   : senselGetNumAvailableLEDs
   * Parameters : handle  : Sensel device handle
   *              num_leds: Pointer to number of leds on device
   * Return     : SENSEL_OK on success or error
   * Description: Retrieve number of LEDs available on the device
   */
  SENSEL_API
  SenselStatus WINAPI senselGetNumAvailableLEDs(SENSEL_HANDLE handle, unsigned char *num_leds);

  /*
   * Function   : senselGetMaxLEDBrightness
   * Parameters : handle        : Sensel device handle
   *              max_brightness: Pointer to maximum per LED brightness
   * Return     : SENSEL_OK on success or error
   * Description: Retrieve the maximum brightness value an LED can be set to
   */
  SENSEL_API
  SenselStatus WINAPI senselGetMaxLEDBrightness(SENSEL_HANDLE handle, unsigned short *max_brightness);

  /*
   * Function   : senselSetLEDBrightness
   * Parameters : handle        : Sensel device handle
   *              led_id        : Index of the LED to update
   *              brightness    : Brightness setting
   * Return     : SENSEL_OK on success or error
   * Description: Update the brightness of one LED
   */
  SENSEL_API
  SenselStatus WINAPI senselSetLEDBrightness(SENSEL_HANDLE handle, unsigned char led_id, unsigned short brightness);

  /*
   * Function   : senselGetLEDBrightness
   * Parameters : handle        : Sensel device handle
   *              led_id        : Index of the LED to update
   *              brightness    : Pointer to brightness setting
   * Return     : SENSEL_OK on success or error
   * Description: Retrieve the current brightness of an LED
   */
  SENSEL_API
  SenselStatus WINAPI senselGetLEDBrightness(SENSEL_HANDLE handle, unsigned char led_id, unsigned short *brightness);

  /*
   * Function   : senselGetPowerButtonPressed
   * Parameters : handle  : Sensel device handle
   *              pressed : Pointer to hold state of power button
   * Return     : SENSEL_OK on success or error
   * Description: Pressed will be 1 if button was pressed or 0 otherwize
   */
  SENSEL_API
  SenselStatus WINAPI senselGetPowerButtonPressed(SENSEL_HANDLE handle, unsigned char *pressed);

	/*
	 * Advanced API
   * The following calls are advanced and can break functionality if not used properly.
	 */

  /*
   * Function   : senselGetFirmwareInfo
   * Parameters : handle : Sensel device handle
   *              fw_info: Pointer to SenselFirmwareInfo structure to populate
   * Return     : SENSEL_OK on success or error
   * Description: Retrieve firmware device information
   */
  SENSEL_API
  SenselStatus WINAPI senselGetFirmwareInfo(SENSEL_HANDLE handle, SenselFirmwareInfo* fw_info);

  /*
   * Function   : senselSetContactsEnableBlobMerge
   * Parameters : handle: Sensel device handle
   *              val   : 0 to disable - 1 to enable
   * Return     : SENSEL_OK on success or error
   * Description: Set contact blob merging setting
   */
  SENSEL_API
  SenselStatus WINAPI senselSetContactsEnableBlobMerge(SENSEL_HANDLE handle, unsigned char val);

  /*
   * Function   : senselGetContactsEnableBlobMerge
   * Parameters : handle: Sensel device handle
   *              val   : Pointer to contain current setting
   * Return     : SENSEL_OK on success or error
   * Description: Get contact blob merging setting
   */
  SENSEL_API
  SenselStatus WINAPI senselGetContactsEnableBlobMerge(SENSEL_HANDLE handle, unsigned char *val);

  /*
   * Function   : senselSetContactsMinForce
   * Parameters : handle: Sensel device handle
   *              val   : Force value
   * Return     : SENSEL_OK on success or error
   * Description: Sets the minimum force a contact needs to have to be reported
   */
  SENSEL_API
  SenselStatus WINAPI senselSetContactsMinForce(SENSEL_HANDLE handle, unsigned short val);

  /*
   * Function   : senselGetContactsMinForce
   * Parameters : handle: Sensel device handle
   *              val   : Pointer to contain current setting
   * Return     : SENSEL_OK on success or error
   * Description: Gets the minimum force a contact needs to have to be reported
   */
  SENSEL_API
  SenselStatus WINAPI senselGetContactsMinForce(SENSEL_HANDLE handle, unsigned short *val);

  /*
   * Function   : senselSetBaselineEnabled
   * Parameters : handle: Sensel device handle
   *              val   : true: Enabled - false: Disabled
   * Return     : SENSEL_OK on success or error
   * Description: Toggles Baselining on or off
   */
  SENSEL_API
  SenselStatus WINAPI senselSetBaselineEnabled(SENSEL_HANDLE handle, unsigned char val);

  /*
   * Function   : senselGetBaselineEnabled
   * Parameters : handle: Sensel device handle
   *              val   : Pointer to contain current setting
   * Return     : SENSEL_OK on success or error
   * Description: Retreives the current baselining setting
   */
  SENSEL_API
  SenselStatus WINAPI senselGetBaselineEnabled(SENSEL_HANDLE handle, unsigned char *val);

  /*
   * Function   : senselSetDynamicBaselineEnabled
   * Parameters : handle: Sensel device handle
   *              val   : true: Enabled - false: Disabled
   * Return     : SENSEL_OK on success or error
   * Description: Sets if the baseline is dynamic and evolves over time
   */
  SENSEL_API
  SenselStatus WINAPI senselSetDynamicBaselineEnabled(SENSEL_HANDLE handle, unsigned char val);

  /*
   * Function   : senselGetDynamicBaselineEnabled
   * Parameters : handle: Sensel device handle
   *              val   : Pointer to contain current setting
   * Return     : SENSEL_OK on success or error
   * Description: Retreives the current dynamic baselining setting
   */
  SENSEL_API
  SenselStatus WINAPI senselGetDynamicBaselineEnabled(SENSEL_HANDLE handle, unsigned char *val);

  /*
   * Function   : senselSetBufferControl
   * Parameters : handle: Sensel device handle
   *              num   : Number of buffers
   * Return     : SENSEL_OK on success or error
   * Description: Sets the number of frame buffers the device should store internaly.
   */
  SENSEL_API
  SenselStatus WINAPI senselSetBufferControl(SENSEL_HANDLE handle, unsigned char num);

  /*
   * Function   : senselGetBufferControl
   * Parameters : handle: Sensel device handle
   *              num   : Pointer to contain current setting
   * Return     : SENSEL_OK on success or error
   * Description: Gets the number of frame buffers the device should store internaly.
   */
  SENSEL_API
  SenselStatus WINAPI senselGetBufferControl(SENSEL_HANDLE handle, unsigned char *num);

  /*
   * Function   : senselSetScanMode
   * Parameters : handle: Sensel device handle
   *              mode  : Scan mode setting
   * Return     : SENSEL_OK on success or error
   * Description: Sets the current scan mode.
   */
  SENSEL_API
  SenselStatus WINAPI senselSetScanMode(SENSEL_HANDLE handle, SenselScanMode mode);

  /*
   * Function   : senselGetScanMode
   * Parameters : handle: Sensel device handle
   *              mode  : Pointer to retrieve Scan mode setting
   * Return     : SENSEL_OK on success or error
   * Description: Gets the current scan mode.
   */
  SENSEL_API
  SenselStatus WINAPI senselGetScanMode(SENSEL_HANDLE handle, SenselScanMode *mode);

  /*
   * Function   : senselSetMaxFrameRate
   * Parameters : handle: Sensel device handle
   *              val   : Max framerate
   * Return     : SENSEL_OK on success or error
   * Description: Sets the maximum framerate at which the device should report
   */
  SENSEL_API
  SenselStatus WINAPI senselSetMaxFrameRate(SENSEL_HANDLE handle, unsigned short val);

  /*
   * Function   : senselGetMaxFrameRate
   * Parameters : handle: Sensel device handle
   *              val   : Pointer to retrieve current max framerate
   * Return     : SENSEL_OK on success or error
   * Description: Gets the maximum framerate at which the device should report
   */
  SENSEL_API
  SenselStatus WINAPI senselGetMaxFrameRate(SENSEL_HANDLE handle, unsigned short *val);

  /*
   * Function   : senselSetContactMask
   * Parameters : handle: Sensel device handle
   *              mask  : Contact information mask
   * Return     : SENSEL_OK on success or error
   * Description: Sets the contact information reported by the sensor
   *              mask can be any combination of CONTACT_MASK_*
   *              Contacts returned in subsequent GetFrame calls will reflect this setting.
   */
  SENSEL_API
  SenselStatus WINAPI senselSetContactsMask(SENSEL_HANDLE handle, unsigned char mask);

  /*
   * Function   : senselGetContactMask
   * Parameters : handle: Sensel device handle
   *              mask  : Pointer to retrieve current max framerate
   * Return     : SENSEL_OK on success or error
   * Description: Gets the current contact mask setting for the device
   */
  SENSEL_API
  SenselStatus WINAPI senselGetContactsMask(SENSEL_HANDLE handle, unsigned char *mask);

  /*
   * Function   : senselReadReg
   * Parameters : handle: Sensel device handle
   *              reg   : Register to read
   *              size  : Size of the register
   *              buf   : Buffer to store the result
   * Return     : SENSEL_OK on success or error
   * Description: Reads size bytes from register reg and stores the value in buf.
   */
  SENSEL_API
  SenselStatus WINAPI senselReadReg(SENSEL_HANDLE handle, unsigned char reg, unsigned char size, unsigned char *buf);

  /*
   * Function   : senselWriteReg
   * Parameters : handle: Sensel device handle
   *              reg   : Register to write
   *              size  : Size of the register
   *              buf   : Buffer containing the data to write
   * Return     : SENSEL_OK on success or error
   * Description: Writes size bytes from buf into register reg
   */
  SENSEL_API
  SenselStatus WINAPI senselWriteReg(SENSEL_HANDLE handle, unsigned char reg, unsigned char size, unsigned char *buf);

  /*
   * Function   : senselReadRegVS
   * Parameters : handle   : Sensel device handle
   *              reg      : Register to write
   *              buf_size : Size of the "buf" buffer
   *              buf      : Buffer to store the result
   *              read_size: Variable to store the number of bytes read from register
   * Return     : SENSEL_OK on success or error
   * Description: Reads up to buf_size bytes from register reg and stores it in buf. On success, read_size will
   *              contain the number of bytes read from the register.
   */
  SENSEL_API
  SenselStatus WINAPI senselReadRegVS(SENSEL_HANDLE handle, unsigned char reg, unsigned int buf_size, unsigned char *buf, unsigned int *read_size);

  /*
   * Function   : senselWriteRegVS
   * Parameters : handle    : Sensel device handle
   *              reg       : Register to write
   *              size      : Size of data to write from buf
   *              buf       : Buffer holding data to write
   *              write_size: Variable to store the number of bytes actually written
   * Return     : SENSEL_OK on success or error
   * Description: Write up to size bytes from buf to register reg. On success, write_size will
   *              contain the number of bytes written to the register.
   */
  SENSEL_API
  SenselStatus WINAPI senselWriteRegVS(SENSEL_HANDLE handle, unsigned char reg, unsigned int size, unsigned char *buf, unsigned int *write_size);

#ifdef __cplusplus
}
#endif

#endif //__SENSEL_H__
