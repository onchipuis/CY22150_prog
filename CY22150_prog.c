/*!
 */
 
// compile using: gcc CY22150_prog.c -I./ -o CY22150_prog -lftd2xx -lMPSSE
// compile using: gcc CY22150_prog.c -I./ -o CY22150_prog.exe -L./ -lftd2xx -lMPSSE

/******************************************************************************/
/* 							 Include files										   */
/******************************************************************************/
/* Standard C libraries */
#include<stdio.h>
#include<stdlib.h>
/* OS specific libraries */
/*#ifdef _WIN32
#include<windows.h>
#endif

#include "WinTypes.h"*/

/* Include D2XX header*/
#include "ftd2xx.h"

/* Include libMPSSE header */
#include "libMPSSE_i2c.h"

/******************************************************************************/
/*								Macro and type defines							   */
/******************************************************************************/
/* Helper macros */

#define APP_CHECK_STATUS(exp) {if(exp!=FT_OK){printf("%s:%d:%s(): status(0x%x) \
!= FT_OK\n",__FILE__, __LINE__, __FUNCTION__,exp);exit(1);}else{;}};
#define CHECK_NULL(exp){if(exp==NULL){printf("%s:%d:%s():  NULL expression \
encountered \n",__FILE__, __LINE__, __FUNCTION__);exit(1);}else{;}};

/* Clock EEPROM parameters*/
#define REF				30000	// Reference clock In KHz
#define IS_ESR_30		1		// Crystal ESR is 30ohm?
#define XCL				12		// Crystal Load Capacitance (in pF)
#define IS_EXTERNAL		0		// 1/0 External Clock?
#define CLKOE_CLK6 		0		// 1/0 Activate (CLK6)
#define CLKOE_CLK5 		0		// 1/0 Activate (CLK5)
#define CLKOE_LCLK4 	0		// 1/0 Activate (LCLK4)
#define CLKOE_LCLK3 	1		// 1/0 Activate (LCLK3)
#define CLKOE_LCLK2 	1		// 1/0 Activate (LCLK2)
#define CLKOE_LCLK1 	1		// 1/0 Activate (LCLK1)
#define DIV1SRC			0		// 1-bit DIV1 Muxer
#define DIV1N			64		// 7-bit DIV1 Divider (Min 4)
#define PB				4		// 10-bit PB Counter
#define PO				0		// 1-bit PO Counter
#define Q				1		// 7-bit Q Counter
#define CLKSRC_LCLK1	1		// 3-bit Crosspoint switch matrix control (LCLK1)
#define CLKSRC_LCLK2	1		// 3-bit Crosspoint switch matrix control (LCLK2)
#define CLKSRC_LCLK3	5		// 3-bit Crosspoint switch matrix control (LCLK3)
#define CLKSRC_LCLK4	0		// 3-bit Crosspoint switch matrix control (LCLK4)
#define CLKSRC_CLK5		0		// 3-bit Crosspoint switch matrix control (CLK5)
#define CLKSRC_CLK6		0		// 3-bit Crosspoint switch matrix control (CLK6)
#define DIV2SRC			0		// 1-bit DIV2 Muxer
#define DIV2N			4		// 7-bit DIV2 Divider (Min 4)

/* Clock EEPROM parameters calculables*/
uint8   XDRV =			0;		// 2-bit Input crystal oscillator drive control
uint8 	CAPLOAD =		0;		// 8-bit Input load capacitor control
uint8	PUMP =			0;		// 3-bit Charge Pump

/* Register definition */
#define REG_09H		((CLKOE_CLK6 << 5) | (CLKOE_CLK5 << 4) | (CLKOE_LCLK4 << 3) | (CLKOE_LCLK3 << 2) | (CLKOE_LCLK2 << 1) | (CLKOE_LCLK1))
#define REG_0CH		((DIV1SRC << 7) | (DIV1N))
#define REG_12H		((1 << 5) | (XDRV << 3))
#define REG_13H		(CAPLOAD)
#define REG_40H		((0xC0) | (PUMP << 2) | ((PB >> 8) & 0x2))
#define REG_41H		(PB & 0xFF)
#define REG_42H		((PO << 7) | (Q))
#define REG_44H		((CLKSRC_LCLK1 << 5) | (CLKSRC_LCLK2 << 2) | ((CLKSRC_LCLK3 >> 1) & 0x2))
#define REG_45H		(((CLKSRC_LCLK3 & 0x1) << 7) | (CLKSRC_LCLK4 << 4) | (CLKSRC_CLK5 << 1) | ((CLKSRC_CLK6 >> 2) & 0x1))
#define REG_46H		(((CLKSRC_CLK6 & 0x2) << 6) | 0x3F)
#define REG_47H		((DIV2SRC << 7) | (DIV2N))

/* Simple Macro-formula definitions*/
#define CapLoad(C_L,C_BRD,C_CHIP)	((C_L-C_BRD-C_CHIP)/0.09375)
#define Q_TOTAL						(Q+2)
#define P_TOTAL						((2*(PB+4)) + PO)

/* Application specific macro definations */
#define I2C_DEVICE_ADDRESS_EEPROM		0x69	// For Clock EEPROM
#define I2C_DEVICE_BUFFER_SIZE			256
#define I2C_WRITE_COMPLETION_RETRY		10

#define RETRY_COUNT_EEPROM		10	/* number of retries if read/write fails */
#define CHANNEL_TO_OPEN			0	/*0 for first available channel, 1 for next... */




/******************************************************************************/
/*								Global variables							  	    */
/******************************************************************************/
uint32 channels;
FT_HANDLE ftHandle;
ChannelConfig channelConf;
FT_STATUS status;
uint8 buffer[I2C_DEVICE_BUFFER_SIZE];

/******************************************************************************/
/*						Public function definitions						  		   */
/******************************************************************************/

uint8 XDRV_det(uint8 bIsExternal, float fFreq, uint8 CEL, uint8 bIsESR30)
{
	fFreq = fFreq / 1000.0f;
	if(bIsExternal)
	{
		if(1.0f <= fFreq && fFreq < 25.0f) return 0;
		else if(25.0f <= fFreq && fFreq < 50.0f) return 1;
		else if(50.0f <= fFreq && fFreq < 90.0f) return 2;
		else if(90.0f <= fFreq && fFreq < 133.0f) return 3;
		else fprintf(stderr, "WARN: There is a unknown frequency when is external ref (%g MHz).\n", fFreq);
	}
	else
	{
		if(8.0f <= fFreq && fFreq < 15.0f) 
		{
			if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 0 : 1);
			else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 1 : 2);
			else if(0xC0 <= CEL) return (bIsESR30 ? 1 : 2);
			else fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when is crystal (%g MHz).\n", CEL, fFreq);
		}
		else if(15.0f <= fFreq && fFreq < 20.0f) 
		{
			if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 1 : 2);
			else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 1 : 2);
			else if(0xC0 <= CEL) return (bIsESR30 ? 2 : 2);
			else fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when is crystal (%g MHz).\n", CEL, fFreq);
		}
		else if(20.0f <= fFreq && fFreq < 25.0f) 
		{
			if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 1 : 2);
			else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 2 : 2);
			else if(0xC0 <= CEL) return (bIsESR30 ? 2 : 3);
			else fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when is crystal (%g MHz).\n", CEL, fFreq);
		}
		else if(25.0f <= fFreq && fFreq <= 30.0f) 
		{
			if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 2 : 2);
			else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 2 : 3);
			else if(0xC0 <= CEL) 
			{	
				if(bIsESR30) return 3;
				else fprintf(stderr, "WARN: N/A in table (%u pF) when is crystal (%g MHz) choosing 60ohm ESR.\n", CEL, fFreq);
			}
			else fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when is crystal (%g MHz).\n", CEL, fFreq);
		}
		else fprintf(stderr, "WARN: There is a unknown frequency when is crystal (%g MHz).\n", fFreq);
	}
	return 0;
}

uint8 CAPLOAD_det(uint8 bIsExternal, float fCapLoad)
{
	if(bIsExternal) return 0;
	else if(0.0 <= fCapLoad && fCapLoad <= 255.0f) return (uint8)((int)fCapLoad);
	else fprintf(stderr, "WARN: There is a unknown CapLoad (%g pF).\n", fCapLoad);
	return 0;
}

int chk_pll(void)
{
	float fVal1 = REF/Q_TOTAL;
	float fVal2 = P_TOTAL*(REF/Q_TOTAL);
	if(fVal1 < 250.0f) {fprintf(stderr, "ERR: PLL stability check 1 failed, REF/Q under 250KHz.\n"); return 0;}
	if(fVal2 > 400000.0f) {fprintf(stderr, "ERR: PLL stability check 2 failed, P*(REF/Q) over 400MHz.\n"); return 0;}
	if(fVal2 < 100000.0f) {fprintf(stderr, "ERR: PLL stability check 3 failed, P*(REF/Q) under 100MHz.\n"); return 0;}
	return 1;
}

int chk_div(void)
{
	if(DIV1N < 4) {fprintf(stderr, "ERR: Div1 check failed, DIV1N under 4.\n"); return 0;}
	if(DIV1N > 127) {fprintf(stderr, "ERR: Div1 check failed, DIV1N over 127.\n"); return 0;}
	if(DIV2N < 4) {fprintf(stderr, "ERR: Div2 check failed, DIV2N under 4.\n"); return 0;}
	if(DIV2N > 127) {fprintf(stderr, "ERR: Div2 check failed, DIV2N over 127.\n"); return 0;}
	return 1;
}

uint8 PUMP_det(void)
{
	int p = P_TOTAL;
	if(16 <= p && p <= 44) return 0;
	else if(45 <= p && p <= 479) return 1;
	else if(480 <= p && p <= 639) return 2;
	else if(640 <= p && p <= 799) return 3;
	else if(800 <= p && p <= 1023) return 4;
	else fprintf(stderr, "WARN: PUMP det failed, P_TOTAL is not in the list (%d).\n", p);
	return 0;
}

float det_out_freq(int clk)
{
	int nCross;
	switch (clk)
	{
		case 1: nCross = CLKSRC_LCLK1; break;
		case 2: nCross = CLKSRC_LCLK2; break;
		case 3: nCross = CLKSRC_LCLK3; break;
		case 4: nCross = CLKSRC_LCLK4; break;
		case 5: nCross = CLKSRC_CLK5; break;
		case 6: nCross = CLKSRC_CLK6; break;
		default: nCross = 0; break;	// WTF?
	}
	float fFreq;
	float fInDiv1 = DIV1SRC?REF:(P_TOTAL*(REF/Q_TOTAL));
	float fInDiv2 = DIV2SRC?REF:(P_TOTAL*(REF/Q_TOTAL));
	switch(nCross)
	{
		case 0: fFreq = REF; break;
		case 1: fFreq = fInDiv1 / DIV1N; break;
		case 2: fFreq = fInDiv1 / 2; 
				if(DIV1N % 4) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be divisible by 4.\n", clk, nCross);
				break;
		case 3: fFreq = fInDiv1 / 3;
				if(DIV1N != 6) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be 6.\n", clk, nCross);  
				break;
		case 4: fFreq = fInDiv2 / DIV2N; break;
		case 5: fFreq = fInDiv2 / 2; 
				if(DIV2N % 4) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be divisible by 4.\n", clk, nCross);  
				break;
		case 6: fFreq = fInDiv2 / 4; 
				if(DIV2N % 8) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be divisible by 8.\n", clk, nCross);  
				break;
		default: fprintf(stderr, "WARN: Bad Cross Point Matrix for CLK%d (%d).\n", clk, nCross); break;
	}
	return fFreq;
}

FT_STATUS write_byte(uint8 slaveAddress, uint8 registerAddress, uint8 data)
{
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

FT_STATUS read_byte(uint8 slaveAddress, uint8 registerAddress, uint8 *data)
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

int main()
{
	FT_STATUS status;
	FT_DEVICE_LIST_INFO_NODE devList;
	uint8 address;
	uint8 data, chkdata;
	int i,j;
	
	/* First, we check all parameters*/
	if(!chk_pll()) return 1;
	if(!chk_div()) return 2;
	CAPLOAD = CAPLOAD_det(IS_EXTERNAL, (float)CapLoad(XCL, 2, 6));
	XDRV = XDRV_det(IS_EXTERNAL, REF, CAPLOAD, IS_ESR_30);
	PUMP = PUMP_det();
	for(i=1; i<=6; i++)
	{
		float fFreq = det_out_freq(i);
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
			printf("		Flags=0x%x\n",devList.Flags);
			printf("		Type=0x%x\n",devList.Type);
			printf("		ID=0x%x\n",devList.ID);
			printf("		LocId=0x%x\n",devList.LocId);
			printf("		SerialNumber=%s\n",devList.SerialNumber);
			printf("		Description=%s\n",devList.Description);
			printf("		FT_STATUSftHandle=0x%x\n",devList.ftHandle);/*is 0 unless open*/
		}

		/* Open the first available channel */
		status = I2C_OpenChannel(CHANNEL_TO_OPEN,&ftHandle);
		APP_CHECK_STATUS(status);
		printf("\nhandle=0x%x status=%d\n",ftHandle,status);
		status = I2C_InitChannel(ftHandle,&channelConf);
		APP_CHECK_STATUS(status);
#define SEND_REG 	status = write_byte(I2C_DEVICE_ADDRESS_EEPROM, address, data); APP_CHECK_STATUS(status);
#define CHK_REG 	status = read_byte(I2C_DEVICE_ADDRESS_EEPROM, address, &chkdata); APP_CHECK_STATUS(status); if(data != chkdata) {fprintf(stderr, "ERR: EEPROM Mistmatch (%x != %x) for address %d", data, chkdata, address); return 1;}
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
