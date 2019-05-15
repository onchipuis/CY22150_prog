// compile using: gcc CY22150_prog.c CY22150_lib.c comm_lib.c -I./ -o CY22150_prog -lftd2xx -lMPSSE
// compile using: gcc CY22150_prog.c CY22150_lib.c comm_lib.c -I./ -o CY22150_prog.exe -L./ -lftd2xx -lMPSSE

/******************************************************************************/
/*                              Include files                                           */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "CY22150_lib.h"
#include "comm_lib.h"

/******************************************************************************/
/*                                Macro and type definitions                               */
/******************************************************************************/

/******************************************************************************/
/*                                Global variables                                      */
/******************************************************************************/
uint32 channels;
FT_HANDLE ftHandle;
ChannelConfig channelConf;

/******************************************************************************/
/*                        Public function definitions                                     */
/******************************************************************************/

int main(int argc, char** argv)
{
    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE devList;
    uint8 address;
    uint8 data, chkdata;
    int i,j;
    
    if(argc >  1) PB =            atoi(argv[ 1]);
    if(argc >  2) PO =            atoi(argv[ 2]);
    if(argc >  3) Q =             atoi(argv[ 3]);
    if(argc >  4) DIV1SRC =       atoi(argv[ 4]);
    if(argc >  5) DIV1N =         atoi(argv[ 5]);
    if(argc >  6) DIV2SRC =       atoi(argv[ 6]);
    if(argc >  7) DIV2N =         atoi(argv[ 7]);
    if(argc >  8) CLKOE_CLK6 =    atoi(argv[ 8]);
    if(argc >  9) CLKOE_CLK5 =    atoi(argv[ 9]);
    if(argc > 10) CLKOE_LCLK4 =   atoi(argv[10]);
    if(argc > 11) CLKOE_LCLK3 =   atoi(argv[11]);
    if(argc > 12) CLKOE_LCLK2 =   atoi(argv[12]);
    if(argc > 13) CLKOE_LCLK1 =   atoi(argv[13]);
    if(argc > 14) CLKSRC_LCLK1 =  atoi(argv[14]);
    if(argc > 15) CLKSRC_LCLK2 =  atoi(argv[15]);
    if(argc > 16) CLKSRC_LCLK3 =  atoi(argv[16]);
    if(argc > 17) CLKSRC_LCLK4 =  atoi(argv[17]);
    if(argc > 18) CLKSRC_CLK5 =   atoi(argv[18]);
    if(argc > 19) CLKSRC_CLK6 =   atoi(argv[19]);
    
    /* First, we check all parameters*/
    if(!chk_pll(1)) return 1;
    if(!chk_div(1)) return 2;
    CAPLOAD = CAPLOAD_det(IS_EXTERNAL, (float)CapLoad(XCL, 2, 6), 1);
    XDRV = XDRV_det(IS_EXTERNAL, REF, CAPLOAD, IS_ESR_30, 1);
    PUMP = PUMP_det(1);
    for(i=1; i<=6; i++)
    {
        float fFreq = det_out_freq(i, 1);
        printf("Frequency for CLK%d = %g KHz\n",i,fFreq);
    }

#ifdef _MSC_VER
    Init_libMPSSE();
#endif
    //channelConf.ClockRate = I2C_CLOCK_FAST_MODE;/*i.e. 400000 KHz*/
    //channelConf.ClockRate = I2C_CLOCK_STANDARD_MODE; /*i.e. 100000 KHz*/
    //channelConf.ClockRate = 20000; /*for test only*/
    channelConf.ClockRate = 2000; /*for test only*/
    channelConf.LatencyTimer= 255;
    //channelConf.Options = I2C_DISABLE_3PHASE_CLOCKING;
    channelConf.Options = I2C_ENABLE_DRIVE_ONLY_ZERO;
    //channelConf.Options = I2C_DISABLE_3PHASE_CLOCKING | I2C_ENABLE_DRIVE_ONLY_ZERO;

    status = I2C_GetNumChannels(&channels);
    APP_CHECK_STATUS(status);
    printf("Number of available I2C channels = %d\n",channels);

    if(channels>0)
    {
        for(i=0;i<channels;i++)
        {
            status = I2C_GetChannelInfo(i,&devList);
            APP_CHECK_STATUS(status);
            printf("Information on channel number %d:\n",i);
            /*print the dev info*/
            printf("        Flags=0x%x\n",devList.Flags);
            printf("        Type=0x%x\n",devList.Type);
            printf("        ID=0x%x\n",devList.ID);
            printf("        LocId=0x%x\n",devList.LocId);
            printf("        SerialNumber=%s\n",devList.SerialNumber);
            printf("        Description=%s\n",devList.Description);
            printf("        FT_STATUSftHandle=0x%x\n",devList.ftHandle);/*is 0 unless open*/
        }

        /* Open the first available channel */
        status = I2C_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
        APP_CHECK_STATUS(status);
        printf("\nhandle=0x%x status=%d\n",ftHandle,status);
        status = I2C_InitChannel(ftHandle,&channelConf);
        APP_CHECK_STATUS(status);
#define SEND_REG     status = write_byte(ftHandle, I2C_DEVICE_ADDRESS_EEPROM, address, data); APP_CHECK_STATUS(status);
#define CHK_REG     status = read_byte(ftHandle, I2C_DEVICE_ADDRESS_EEPROM, address, &chkdata); APP_CHECK_STATUS(status); if(data != chkdata) {fprintf(stderr, "ERR: EEPROM Mistmatch (%x != %x) for address %d", data, chkdata, address); return 1;}
        address = 0x09; data = REG_09H; SEND_REG CHK_REG
        address = 0x0C; data = REG_0CH; SEND_REG CHK_REG
        address = 0x12; data = REG_12H; SEND_REG CHK_REG
        address = 0x13; data = REG_13H; SEND_REG CHK_REG
        address = 0x40; data = REG_40H; SEND_REG CHK_REG
        address = 0x41; data = REG_41H; SEND_REG CHK_REG
        address = 0x42; data = REG_42H; SEND_REG CHK_REG
        address = 0x44; data = REG_44H; SEND_REG CHK_REG
        address = 0x45; data = REG_45H; SEND_REG CHK_REG
        address = 0x46; data = REG_46H; SEND_REG CHK_REG
        address = 0x47; data = REG_47H; SEND_REG CHK_REG
        
        status = I2C_CloseChannel(ftHandle);
    }

#ifdef _MSC_VER
    Cleanup_libMPSSE();
#endif

    return 0;
}
