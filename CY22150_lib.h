#ifndef CY22150_LIB_H
#define CY22150_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

extern int REF;   // Reference clock In kHz
extern int IS_ESR_30;       // Crystal ESR is 30ohm?
extern int XCL;      // Crystal Load Capacitance (in pF)
extern int IS_EXTERNAL;       // 1/0 External Clock?
extern int CLKOE_CLK6;       // 1/0 Activate (CLK6)
extern int CLKOE_CLK5;       // 1/0 Activate (CLK5)
extern int CLKOE_LCLK4;       // 1/0 Activate (LCLK4)
extern int CLKOE_LCLK3;       // 1/0 Activate (LCLK3)
extern int CLKOE_LCLK2;       // 1/0 Activate (LCLK2)
extern int CLKOE_LCLK1;       // 1/0 Activate (LCLK1)
extern int DIV1SRC;       // 1-bit DIV1 Muxer
extern int DIV1N;      // 7-bit DIV1 Divider (Min 4)
extern int PB;       // 10-bit PB Counter
extern int PO;       // 1-bit PO Counter
extern int Q;       // 7-bit Q Counter
extern int CLKSRC_LCLK1;       // 3-bit Crosspoint switch matrix control (LCLK1)
extern int CLKSRC_LCLK2;       // 3-bit Crosspoint switch matrix control (LCLK2)
extern int CLKSRC_LCLK3;       // 3-bit Crosspoint switch matrix control (LCLK3)
extern int CLKSRC_LCLK4;       // 3-bit Crosspoint switch matrix control (LCLK4)
extern int CLKSRC_CLK5;       // 3-bit Crosspoint switch matrix control (CLK5)
extern int CLKSRC_CLK6;       // 3-bit Crosspoint switch matrix control (CLK6)
extern int DIV2SRC;       // 1-bit DIV2 Muxer
extern int DIV2N;       // 7-bit DIV2 Divider (Min 4)
extern uint8_t   XDRV;        // 2-bit Input crystal oscillator drive control
extern uint8_t   CAPLOAD;        // 8-bit Input load capacitor control
extern uint8_t   PUMP;        // 3-bit Charge Pump

/* Register definition */
#define REG_09H        ((CLKOE_CLK6 << 5) | (CLKOE_CLK5 << 4) | (CLKOE_LCLK4 << 3) | (CLKOE_LCLK3 << 2) | (CLKOE_LCLK2 << 1) | (CLKOE_LCLK1))
#define REG_0CH        ((DIV1SRC << 7) | (DIV1N))
#define REG_12H        ((1 << 5) | (XDRV << 3))
#define REG_13H        (CAPLOAD)
#define REG_40H        ((0xC0) | (PUMP << 2) | ((PB >> 8) & 0x2))
#define REG_41H        (PB & 0xFF)
#define REG_42H        ((PO << 7) | (Q))
#define REG_44H        ((CLKSRC_LCLK1 << 5) | (CLKSRC_LCLK2 << 2) | ((CLKSRC_LCLK3 >> 1) & 0x2))
#define REG_45H        (((CLKSRC_LCLK3 & 0x1) << 7) | (CLKSRC_LCLK4 << 4) | (CLKSRC_CLK5 << 1) | ((CLKSRC_CLK6 >> 2) & 0x1))
#define REG_46H        (((CLKSRC_CLK6 & 0x2) << 6) | 0x3F)
#define REG_47H        ((DIV2SRC << 7) | (DIV2N))

/* Simple Macro-formula definitions*/
#define CapLoad(C_L,C_BRD,C_CHIP)       ((C_L-C_BRD-C_CHIP)/0.09375)
#define Q_TOTAL                         (Q+2)
#define P_TOTAL                         ((2*(PB+4)) + PO)

uint8_t XDRV_det(uint8_t bIsExternal, float fFreq, uint8_t CEL, uint8_t bIsESR30, char print);
uint8_t CAPLOAD_det(uint8_t bIsExternal, float fCapLoad, char print);
int chk_pll(char print);
int chk_div(char print);
uint8_t PUMP_det(char print);
float det_out_freq(int clk, char print);

#ifdef __cplusplus
}
#endif

#endif
