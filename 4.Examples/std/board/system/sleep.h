#ifndef __SLEEP_H__
#define __SLEEP_H__

#include "stm32f4xx.h"

#define dwt_freq()

typedef enum {
    UNIT_US = 1ul,
    UNIT_MS = 1000ul,
    UNIT_S  = 1000000ul,
} DelayUnit_e;

void dwt_init(void);
void dwt_wait(u32 us);

#define sleep(us)   dwt_wait(us)

#define sleep_us(t) sleep((t) * (UNIT_US))
#define sleep_ms(t) sleep((t) * (UNIT_MS))
#define sleep_s(t)  sleep((t) * (UNIT_S))

u32 GetSysClk(void);

#endif
