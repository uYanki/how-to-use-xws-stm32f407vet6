#ifndef __SLEEP_H__
#define __SLEEP_H__

#include <stdbool.h>
#include "stm32f4xx.h"

//---------------------------------------------------------------
//

typedef enum {
    UNIT_US = 1ul,
    UNIT_MS = 1000ul,
    UNIT_S  = 1000000ul,
} DelayUnit_e;

#define sleep_init() dwt_init()
#define sleep(us)    dwt_wait(us)

#define sleep_us(t)  sleep((t) * (UNIT_US))
#define sleep_ms(t)  sleep((t) * (UNIT_MS))
#define sleep_s(t)   sleep((t) * (UNIT_S))

inline void dwt_init(void);
inline void dwt_wait(u32 us);

//---------------------------------------------------------------
//

inline void DelayInit(void);
inline void DelayBlock(u32 nWaitTime);
inline bool DelayNonBlock(u32 nStartTick, u32 nWaitTime);

inline void HAL_IncTick(void);
inline u32  HAL_GetTick(void);  // unit: ms

#define DelayBlockMS(t)       DelayBlock((t) * (UNIT_MS))
#define DelayBlockS(t)        DelayBlock((t) * (UNIT_S))

#define DelayNonBlockMS(s, t) DelayNonBlock(s, (t) * (UNIT_MS))
#define DelayNonBlockS(s, t)  DelayNonBlock(s, (t) * (UNIT_S))

//---------------------------------------------------------------
//

void FirewareDelay(u32 nWaitTime);

#endif
