/*
I2C Memory communication library
*/
#include <stdio.h>
#include <stdlib.h>
#include "comm_lib.h"

uint8 buffer[I2C_DEVICE_BUFFER_SIZE];
FT_STATUS write_byte(FT_HANDLE ftHandle, uint8 slaveAddress, uint8 registerAddress, uint8 data)
{
    FT_STATUS status;
    uint32 bytesToTransfer = 0;
    uint32 bytesTransfered;
    bool writeComplete=0;
    uint32 retry=0;

    bytesToTransfer=0;
    bytesTransfered=0;
    buffer[bytesToTransfer++]=registerAddress; // Byte addressed inside EEPROM 
    buffer[bytesToTransfer++]=data;
    status = I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, \
&bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT|I2C_TRANSFER_OPTIONS_STOP_BIT);
    APP_CHECK_STATUS(status);

    // ACK not available

    // poll to check completition 
    while((writeComplete==0) && (retry<I2C_WRITE_COMPLETION_RETRY))
    {
        bytesToTransfer=0;
        bytesTransfered=0;
        buffer[bytesToTransfer++]=registerAddress; // Addressed inside EEPROM  
        status = I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer,\
            buffer, &bytesTransfered, \
            I2C_TRANSFER_OPTIONS_START_BIT|I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
        if((FT_OK == status) && (bytesToTransfer == bytesTransfered))
        {
            writeComplete=1;
            printf("  ... Write done\n");
        }
        retry++;
        //printf("Retry=%d\n",retry);
    }
    return status;
}

FT_STATUS read_byte(FT_HANDLE ftHandle, uint8 slaveAddress, uint8 registerAddress, uint8 *data)
{
    FT_STATUS status;
    uint32 bytesToTransfer = 0;
    uint32 bytesTransfered;

    bytesToTransfer=0;
    bytesTransfered=0;
    buffer[bytesToTransfer++]=registerAddress; /* Byte addressed inside EEPROM */
    status = I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, \
        &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT);
    APP_CHECK_STATUS(status);
    bytesToTransfer=1;
    bytesTransfered=0;
    status |= I2C_DeviceRead(ftHandle, slaveAddress, bytesToTransfer, buffer, \
        &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT);
    APP_CHECK_STATUS(status);
    *data = buffer[0];
    return status;
}

