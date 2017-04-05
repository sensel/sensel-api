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

#include <stdio.h>
#include "sensel_serial.h"
#include "sensel.h"

extern int cSendData(unsigned char* bytes, int len);
extern int cReadData(unsigned char* bytes, int len);
extern int cGetAvailable();
extern int cOpenBLE();


bool senselSerialWrite(SenselSerialHandle *data, uint8 *buf, int buf_len)
{
    // TODO: Handle writes over 20 bytes
    cSendData(buf, buf_len);
    /*
    int wr = write(data->serial_fd, buf, buf_len);
    
    //THIS CAUSES READ FAILURE IN MAC OSX!!! tcflush(data->serial_fd, TCOFLUSH);
    
    if(wr == -1)
    {
        perror("FAILED TO WRITE TO DEVICE");
        return false;
    }
    */
    return true;
}

int senselSerialReadAvailable(SenselSerialHandle *data, uint8 *buf, int buf_len)
{
    return cReadData(buf, buf_len);
    /*
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(data->serial_fd, &read_fds);
    
    //select() changes timeout value, so we need this on every call to readAvailable
    struct timeval timeout;
    timeout.tv_sec = SENSEL_SERIAL_TIMEOUT_SEC;
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
        printf("select() timedout with no data, ret=%d\n", ret);
        return 0;
    }
    */
}

// Checks number of available bytes
int  senselSerialGetAvailable(SenselSerialHandle *data)
{
  return cGetAvailable();
}

// TODO: When we do serial over BLE, we just do an infinite loop while waiting for BLE
// data. This is not good. Need a better solution!
bool senselSerialReadBytes(SenselSerialHandle *data, uint8 *buf, int buf_len)
{
    int max_attempts = 1;
    int failed_attempts = 0;
    int bytes_left = buf_len;
    
    do
    {
        int bytes_read = senselSerialReadAvailable(data, buf, bytes_left);
        
        if(bytes_read <= 0)
        {
          // TODO: This is bad... right now, we spin infinitely until we receive data!!!
            //failed_attempts++;
        }
        else
        {
            buf += bytes_read;
            bytes_left -= bytes_read;
        }
        
        if(failed_attempts >= max_attempts)
        {
            return false;
        }
    }
    while(bytes_left > 0);
    
    return true;
}

void senselSerialFlushInput(SenselSerialHandle *data)
{
    /*
    int bytes_read = 0;
    int byte_total = 0;
    unsigned char buf[128];
    
    //Read while there are bytes available
    do
    {
        bytes_read = senselSerialReadAvailable(data, buf, 128);
        byte_total += bytes_read;
    }
    while(bytes_read > 0);
    
    printf("Cleared %d bytes from buffer!\n", byte_total);
    */
}

/*
bool senselSerialOpen2(SenselSerialHandle * data, char* file_name)
{
    char magic[7];
    magic[6] = '\0';
    
    printf("Opening %s...", file_name);
    
    data->serial_fd = open(file_name, O_RDWR | O_NONBLOCK | O_NOCTTY);
    
    if(data->serial_fd == -1)
    {
        printf("unable to open!\n");
        return false;
    }
    
    struct termios options;
    tcgetattr(data->serial_fd, &options);
    cfmakeraw(&options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    tcsetattr(data->serial_fd, TCSANOW, &options);
    
    senselSerialFlushInput(data);
    
    if(senselReadReg(0x00, SENSEL_MAGIC_LEN, (uint8*)magic))
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
    
    close(data->serial_fd);
    data->serial_fd = -1;
    return false;
}
 */

bool senselSerialOpen(SenselSerialHandle * data, char* com_port)
{
    if(cOpenBLE())
        return true;
    else
        return false;
    /*
    DIR *d;
    struct dirent *dir;
    char file_name[100];
    bool found_sensor = false;
    
    if(com_port != NULL)
    {
        return senselSerialOpen2(data, com_port);
    }
    
    d = opendir(SENSEL_SERIAL_DIR);
    if(!d)
    {
        printf("Could not open %s directory.", SENSEL_SERIAL_DIR);
        return false;
    }
    
    while((dir = readdir(d)) != 0)
    {
        strcpy(file_name, SENSEL_SERIAL_DIR);
        
        if(strstr(dir->d_name, "ttyS") ||
           strstr(dir->d_name, "ttyUSB") ||
           strstr(dir->d_name, "cu.usbmodem") || 
           strstr(dir->d_name, "ttyACM"))
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
    */
}

void senselSerialClose(SenselSerialHandle *data)
{
    /*
    if(data->serial_fd != -1)
    {
        close(data->serial_fd);
        data->serial_fd = -1;
    }
    */
}
