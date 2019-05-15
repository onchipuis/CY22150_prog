#ifndef COMM_LIB_H
#define COMM_LIB_H

/* OS specific libraries */
/*#ifdef _WIN32
#include<windows.h>
#endif

#include "WinTypes.h"*/

/* Include D2XX header*/
#include "ftd2xx.h"

/* Include libMPSSE header */

#include "libMPSSE_i2c.h"

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};

#define I2C_DEVICE_ADDRESS_EEPROM       0x69    // For Clock EEPROM
#define I2C_DEVICE_BUFFER_SIZE          256
#define I2C_WRITE_COMPLETION_RETRY      10

#define RETRY_COUNT_EEPROM              10    /* number of retries if read/write fails */
#define CHANNEL_TO_OPEN                 0    /*0 for first available channel, 1 for next... */

FT_STATUS write_byte(FT_HANDLE ftHandle, uint8 slaveAddress, uint8 registerAddress, uint8 data);
FT_STATUS read_byte(FT_HANDLE ftHandle, uint8 slaveAddress, uint8 registerAddress, uint8 *data);

#endif
