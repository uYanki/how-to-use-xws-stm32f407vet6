#ifndef __DWT_H__
#define __DWT_H__

#include "stm32f4xx.h"

#define dwt_freq()    (SystemCoreClock)

// read timestamp
#define DWT_Read_TS() DWT_CYCCNT

typedef enum {
    UNIT_US = 1ul,
    UNIT_MS = 1000ul,
    UNIT_S  = 1000000ul,
} DelayUnit_e;

#define DWT_Wait(time, unit) DWT_WaitUS((time) * (unit))

void DWT_Init(void);
void DWT_WaitUS(uint32_t nus);

u32 GetSysClk(void);

#endif
