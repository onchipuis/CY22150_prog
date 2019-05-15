// compile using: g++ CY22150_freq_explorer.cpp CY22150_lib.c -I./ -o CY22150_freq_explorer
// compile using: g++ CY22150_freq_explorer.cpp CY22150_lib.c -I./ -o CY22150_freq_explorer.exe

#include "CY22150_lib.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctype.h>
#include <vector>

using namespace std;
#define PRINT 0

struct CONFIG_CY22150 { 
  int CLKOE_CLK6;
  int CLKOE_CLK5;
  int CLKOE_LCLK4;
  int CLKOE_LCLK3;
  int CLKOE_LCLK2;
  int CLKOE_LCLK1;
  int DIV1SRC;
  int DIV1N;
  int PB;
  int PO;
  int Q;
  int CLKSRC_LCLK1;
  int CLKSRC_LCLK2;
  int CLKSRC_LCLK3;
  int CLKSRC_LCLK4;
  int CLKSRC_CLK5;
  int CLKSRC_CLK6;
  int DIV2SRC;
  int DIV2N;
  float fFreq;
};

vector<CONFIG_CY22150> configs;

int calculate_and_output() {
  if(!chk_pll(PRINT)) return 0;
  if(!chk_div(PRINT)) return 0;
  CAPLOAD = CAPLOAD_det(IS_EXTERNAL, (float)CapLoad(XCL, 2, 6), PRINT);
  XDRV = XDRV_det(IS_EXTERNAL, REF, CAPLOAD, IS_ESR_30, PRINT);
  PUMP = PUMP_det(PRINT);
  float fFreq = det_out_freq(1, PRINT);
  configs.push_back(
    (CONFIG_CY22150){
      CLKOE_CLK6,
      CLKOE_CLK5,
      CLKOE_LCLK4,
      CLKOE_LCLK3,
      CLKOE_LCLK2,
      CLKOE_LCLK1,
      DIV1SRC,
      DIV1N,
      PB,
      PO,
      Q,
      CLKSRC_LCLK1,
      CLKSRC_LCLK2,
      CLKSRC_LCLK3,
      CLKSRC_LCLK4,
      CLKSRC_CLK5,
      CLKSRC_CLK6,
      DIV2SRC,
      DIV2N,
      fFreq});
  return 1;
}

int iterate_internal() {
  // Div in 2
  DIV1N = 4;
  CLKSRC_LCLK1 = 2;
  calculate_and_output();
  
  // DIV in 3
  DIV1N = 6;
  CLKSRC_LCLK1 = 3;
  calculate_and_output();
  
  // Div in 4 ~ 127
  CLKSRC_LCLK1 = 1;
  for(DIV1N = 4; DIV1N < 128; DIV1N++) {
    calculate_and_output();
  }
}

int main(int argc, char** argv) {
  printf("Calculating frequencies...\n");
  // With DIV1SRC in VCO
  DIV1SRC = 0;
  for(Q = 0; Q < (1 << 7); Q++) {
    for(PB = 4; PB < (1 << 10); PB++) {
      for(PO = 0; PO < 2; PO++) {
        iterate_internal();
      }
    }
  }
  // With DIV1SRC in REF
  PB = 4;
  PO = 0;
  Q =  1;
  DIV1SRC = 1;
  iterate_internal();
  
  for(;;)
  {
    float fFreq = 0.0;
    printf("Frequency? [in KHz]: ");
    while(scanf("%g", &fFreq) != 1) {
      printf("Not valid.\n");
      char c = '0';
      do {
        c = getchar();
      }
      while (!isdigit(c));
      ungetc(c, stdin);
    }
    float fdif = fabs(configs.begin()->fFreq - fFreq);
    vector<CONFIG_CY22150>::iterator best = configs.begin();
    
    for(vector<CONFIG_CY22150>::iterator it = configs.begin(); it != configs.end(); it++) {
      float dif = fabs(it->fFreq - fFreq);
      if(dif < fdif) {
        fdif = dif;
        best = it;
      }
    }
    
    PB = best->PB;
    PO = best->PO;
    Q = best->Q;
    DIV1SRC = best->DIV1SRC;
    DIV1N = best->DIV1N;
    DIV2SRC = best->DIV2SRC;
    DIV2N = best->DIV2N;
    CLKOE_CLK6 = best->CLKOE_CLK6;
    CLKOE_CLK5 = best->CLKOE_CLK5;
    CLKOE_LCLK4 = best->CLKOE_LCLK4;
    CLKOE_LCLK3 = best->CLKOE_LCLK3;
    CLKOE_LCLK2 = best->CLKOE_LCLK2;
    CLKOE_LCLK1 = best->CLKOE_LCLK1;
    CLKSRC_LCLK1 = best->CLKSRC_LCLK1;
    CLKSRC_LCLK2 = best->CLKSRC_LCLK2;
    CLKSRC_LCLK3 = best->CLKSRC_LCLK3;
    CLKSRC_LCLK4 = best->CLKSRC_LCLK4;
    CLKSRC_CLK5 = best->CLKSRC_CLK5;
    CLKSRC_CLK6 = best->CLKSRC_CLK6;
    
    printf("Best match: \n");
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %g\n", 
        PB,
        PO,
        Q,
        DIV1SRC,
        DIV1N,
        DIV2SRC,
        DIV2N,
        CLKOE_CLK6,
        CLKOE_CLK5,
        CLKOE_LCLK4,
        CLKOE_LCLK3,
        CLKOE_LCLK2,
        CLKOE_LCLK1,
        CLKSRC_LCLK1,
        CLKSRC_LCLK2,
        CLKSRC_LCLK3,
        CLKSRC_LCLK4,
        CLKSRC_CLK5,
        CLKSRC_CLK6,
        best->fFreq);
    printf("Checkings:\n");
    if(chk_pll(1)) if(chk_div(1)) {
      CAPLOAD = CAPLOAD_det(IS_EXTERNAL, (float)CapLoad(XCL, 2, 6), 1);
      XDRV = XDRV_det(IS_EXTERNAL, REF, CAPLOAD, IS_ESR_30, 1);
      PUMP = PUMP_det(1);
    }
    printf("Invoking...\n");
    char cmd[256];
    sprintf(cmd, "./CY22150_prog %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %g", 
        PB,
        PO,
        Q,
        DIV1SRC,
        DIV1N,
        DIV2SRC,
        DIV2N,
        CLKOE_CLK6,
        CLKOE_CLK5,
        CLKOE_LCLK4,
        CLKOE_LCLK3,
        CLKOE_LCLK2,
        CLKOE_LCLK1,
        CLKSRC_LCLK1,
        CLKSRC_LCLK2,
        CLKSRC_LCLK3,
        CLKSRC_LCLK4,
        CLKSRC_CLK5,
        CLKSRC_CLK6);
    system(cmd);
  }
  
  return 0;
}


