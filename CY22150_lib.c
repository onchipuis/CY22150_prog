// compile using: gcc CY22150_prog.c -I./ -o CY22150_prog -lftd2xx -lMPSSE
// compile using: gcc CY22150_prog.c -I./ -o CY22150_prog.exe -L./ -lftd2xx -lMPSSE

/******************************************************************************/
/*                              Include files                                 */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "CY22150_lib.h"

/******************************************************************************/
/*                                Macro and type definitions                               */
/******************************************************************************/

/* Clock EEPROM parameters*/
int REF =               30000;   // Reference clock In kHz
int IS_ESR_30 =         1;       // Crystal ESR is 30ohm?
int XCL =               12;      // Crystal Load Capacitance (in pF)
int IS_EXTERNAL =       0;       // 1/0 External Clock?
int CLKOE_CLK6 =        0;       // 1/0 Activate (CLK6)
int CLKOE_CLK5 =        0;       // 1/0 Activate (CLK5)
int CLKOE_LCLK4 =       0;       // 1/0 Activate (LCLK4)
int CLKOE_LCLK3 =       1;       // 1/0 Activate (LCLK3)
int CLKOE_LCLK2 =       1;       // 1/0 Activate (LCLK2)
int CLKOE_LCLK1 =       1;       // 1/0 Activate (LCLK1)
int DIV1SRC =           0;       // 1-bit DIV1 Muxer
int DIV1N =             64;      // 7-bit DIV1 Divider (Min 4)
int PB =                4;       // 10-bit PB Counter
int PO =                0;       // 1-bit PO Counter
int Q =                 1;       // 7-bit Q Counter
int CLKSRC_LCLK1 =      1;       // 3-bit Crosspoint switch matrix control (LCLK1)
int CLKSRC_LCLK2 =      1;       // 3-bit Crosspoint switch matrix control (LCLK2)
int CLKSRC_LCLK3 =      5;       // 3-bit Crosspoint switch matrix control (LCLK3)
int CLKSRC_LCLK4 =      0;       // 3-bit Crosspoint switch matrix control (LCLK4)
int CLKSRC_CLK5 =       0;       // 3-bit Crosspoint switch matrix control (CLK5)
int CLKSRC_CLK6 =       0;       // 3-bit Crosspoint switch matrix control (CLK6)
int DIV2SRC =           0;       // 1-bit DIV2 Muxer
int DIV2N =             4;       // 7-bit DIV2 Divider (Min 4)

/* Clock EEPROM parameters calculables*/
uint8_t   XDRV =        0;        // 2-bit Input crystal oscillator drive control
uint8_t   CAPLOAD =     0;        // 8-bit Input load capacitor control
uint8_t   PUMP =        0;        // 3-bit Charge Pump


/******************************************************************************/
/*                                Global variables                                      */
/******************************************************************************/

/******************************************************************************/
/*                        Public function definitions                                     */
/******************************************************************************/

uint8_t XDRV_det(uint8_t bIsExternal, float fFreq, uint8_t CEL, uint8_t bIsESR30, char print)
{
    fFreq = fFreq / 1000.0f;
    if(bIsExternal)
    {
        if(1.0f <= fFreq && fFreq < 25.0f) return 0;
        else if(25.0f <= fFreq && fFreq < 50.0f) return 1;
        else if(50.0f <= fFreq && fFreq < 90.0f) return 2;
        else if(90.0f <= fFreq && fFreq < 133.0f) return 3;
        else if(print) fprintf(stderr, "WARN: There is a unknown frequency when it is external ref (%g MHz).\n", fFreq);
    }
    else
    {
        if(8.0f <= fFreq && fFreq < 15.0f) 
        {
            if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 0 : 1);
            else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 1 : 2);
            else if(0xC0 <= CEL) return (bIsESR30 ? 1 : 2);
            else if(print) fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when it is crystal (%g MHz).\n", CEL, fFreq);
        }
        else if(15.0f <= fFreq && fFreq < 20.0f) 
        {
            if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 1 : 2);
            else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 1 : 2);
            else if(0xC0 <= CEL) return (bIsESR30 ? 2 : 2);
            else if(print) fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when it is crystal (%g MHz).\n", CEL, fFreq);
        }
        else if(20.0f <= fFreq && fFreq < 25.0f) 
        {
            if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 1 : 2);
            else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 2 : 2);
            else if(0xC0 <= CEL) return (bIsESR30 ? 2 : 3);
            else fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when it is crystal (%g MHz).\n", CEL, fFreq);
        }
        else if(25.0f <= fFreq && fFreq <= 30.0f) 
        {
            if(0x00 <= CEL && CEL < 0x80) return (bIsESR30 ? 2 : 2);
            else if(0x80 <= CEL && CEL < 0xC0) return (bIsESR30 ? 2 : 3);
            else if(0xC0 <= CEL) 
            {    
                if(bIsESR30) return 3;
                else if(print) fprintf(stderr, "WARN: N/A in table (%u pF) when it is crystal (%g MHz) choosing 60ohm ESR.\n", CEL, fFreq);
            }
            else if(print) fprintf(stderr, "WARN: There is a unknown CapLoad (%u pF) when it is crystal (%g MHz).\n", CEL, fFreq);
        }
        else if(print) fprintf(stderr, "WARN: There is a unknown frequency when it is crystal (%g MHz).\n", fFreq);
    }
    return 0;
}

uint8_t CAPLOAD_det(uint8_t bIsExternal, float fCapLoad, char print)
{
    if(bIsExternal) return 0;
    else if(0.0 <= fCapLoad && fCapLoad <= 255.0f) return (uint8_t)((int)fCapLoad);
    else if(print) fprintf(stderr, "WARN: There is a unknown CapLoad (%g pF).\n", fCapLoad);
    return 0;
}

int chk_pll(char print)
{
    float fVal1 = (float)REF/(float)Q_TOTAL;
    float fVal2 = (float)P_TOTAL*((float)REF/(float)Q_TOTAL);
    if(fVal1 < 250.0f) { if(print) fprintf(stderr, "ERR: PLL stability check 1 failed, REF/Q under 250KHz.\n"); return 0;}
    if(fVal2 > 400000.0f) { if(print) fprintf(stderr, "ERR: PLL stability check 2 failed, P*(REF/Q) over 400MHz.\n"); return 0;}
    if(fVal2 < 100000.0f) { if(print) fprintf(stderr, "ERR: PLL stability check 3 failed, P*(REF/Q) under 100MHz.\n"); return 0;}
    return 1;
}

int chk_div(char print)
{
    if(DIV1N < 4) { if(print) fprintf(stderr, "ERR: Div1 check failed, DIV1N under 4.\n"); return 0;}
    if(DIV1N > 127) { if(print) fprintf(stderr, "ERR: Div1 check failed, DIV1N over 127.\n"); return 0;}
    if(DIV2N < 4) { if(print)fprintf(stderr, "ERR: Div2 check failed, DIV2N under 4.\n"); return 0;}
    if(DIV2N > 127) { if(print)fprintf(stderr, "ERR: Div2 check failed, DIV2N over 127.\n"); return 0;}
    return 1;
}

uint8_t PUMP_det(char print)
{
    int p = P_TOTAL;
    if(16 <= p && p <= 44) return 0;
    else if(45 <= p && p <= 479) return 1;
    else if(480 <= p && p <= 639) return 2;
    else if(640 <= p && p <= 799) return 3;
    else if(800 <= p && p <= 1023) return 4;
    else if(print) fprintf(stderr, "WARN: PUMP det failed, P_TOTAL is not in the list (%d).\n", p);
    return 0;
}

float det_out_freq(int clk, char print)
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
        default: nCross = 0; break;    // WTF?
    }
    float fFreq;
    float fInDiv1 = DIV1SRC?REF:((float)P_TOTAL*((float)REF/(float)Q_TOTAL));
    float fInDiv2 = DIV2SRC?REF:((float)P_TOTAL*((float)REF/(float)Q_TOTAL));
    switch(nCross)
    {
        case 0: fFreq = REF; break;
        case 1: fFreq = fInDiv1 / DIV1N; break;
        case 2: fFreq = fInDiv1 / 2; 
                if(DIV1N % 4) if(print) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be divisible by 4.\n", clk, nCross);
                break;
        case 3: fFreq = fInDiv1 / 3;
                if(DIV1N != 6) if(print) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be 6.\n", clk, nCross);  
                break;
        case 4: fFreq = fInDiv2 / DIV2N; break;
        case 5: fFreq = fInDiv2 / 2; 
                if(DIV2N % 4) if(print) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be divisible by 4.\n", clk, nCross);  
                break;
        case 6: fFreq = fInDiv2 / 4; 
                if(DIV2N % 8) if(print) fprintf(stderr, "WARN: For CLK%d, CLKSRC%d: DIV1N must be divisible by 8.\n", clk, nCross);  
                break;
        default: if(print) fprintf(stderr, "WARN: Bad Cross Point Matrix for CLK%d (%d).\n", clk, nCross); break;
    }
    return fFreq;
}

