#ifndef __USDK_DELAY_H__
#define __USDK_DELAY_H__

#include "stm32f4xx.h"

typedef enum {
    UNIT_US = 1ul,
    UNIT_MS = 1000ul,
    UNIT_S  = 1000000ul,
} DelayUnit_e;

typedef uint32_t tick_t;
#define TICK_MAX (UINT32_MAX)

void   DelayInit(void);
void   DelayBlock(tick_t us);
tick_t DelayGetTick(void);
tick_t DelayCalcDelta(tick_t nStartTick, tick_t nEndTick);

#define DelayBlockUS(t)       DelayBlock((t) * (UNIT_US))
#define DelayNonBlockUS(s, t) DelayNonBlock(s, (t) * (UNIT_US))

#define DelayBlockMS(t)       DelayBlock((t) * (UNIT_MS))
#define DelayNonBlockMS(s, t) DelayNonBlock(s, (t) * (UNIT_MS))

#define DelayBlockS(t)        DelayBlock((t) * (UNIT_S))
#define DelayNonBlockS(s, t)  DelayNonBlock(s, (t) * (UNIT_S))

#endif
