#ifndef __SLEEP_H__
#define __SLEEP_H__

#include "stm32f4xx.h"

typedef enum {
    UNIT_US = 1ul,
    UNIT_MS = 1000ul,
    UNIT_S  = 1000000ul,
} DelayUnit_e;

void dwt_init(void);
void dwt_wait(u32 us);
u32  dwt_tick(void);

void tim_init(void);
void tim_wait(u32 us);

#if 1
#define sleep_init() dwt_init()
#define sleep(us)    dwt_wait(us)
#else
#define sleep_init() tim_init()
#define sleep(us)    tim_wait(us)
#endif

#define sleep_us(t) sleep((t) * (UNIT_US))
#define sleep_ms(t) sleep((t) * (UNIT_MS))
#define sleep_s(t)  sleep((t) * (UNIT_S))

void FirewareDelay(u32 nWaitTime);

#endif
