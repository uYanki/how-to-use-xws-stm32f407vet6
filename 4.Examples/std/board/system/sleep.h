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

//---------------------------------------------------------------
//

#define sleep_init() dwt_init()
#define sleep(us)    dwt_wait(us)

#define sleep_us(t)  sleep((t) * (UNIT_US))
#define sleep_ms(t)  sleep((t) * (UNIT_MS))
#define sleep_s(t)   sleep((t) * (UNIT_S))

inline void dwt_init(void);
inline void dwt_wait(u32 us);
inline u32  dwt_tick(void);

//---------------------------------------------------------------
//

#if 1
typedef u32 tick_t;
#define TICK_MAX (UINT32_MAX)
#else
typedef u64 tick_t;
#define TICK_MAX (UINT64_MAX)
#endif

// increment
#ifndef TICK_INC
#define TICK_INC (UNIT_MS)
#endif

// prescaler
#ifndef TICK_PSC
#define TICK_PSC (UNIT_MS)
#endif

inline void DelayInit(void);
inline void DelayBlock(tick_t nWaitTime);
inline bool DelayNonBlock(tick_t nStartTick, tick_t nWaitTime);

inline void   HAL_IncTick(void);
inline tick_t HAL_GetTick(void);
inline tick_t HAL_DeltaTick(tick_t nStartTick, tick_t nEndTick);

// #define DelayBlockUS(t)       DelayBlock((t) * (UNIT_US))
#define DelayBlockMS(t)       DelayBlock((t) * (UNIT_MS))
#define DelayBlockS(t)        DelayBlock((t) * (UNIT_S))

// #define DelayNonBlockUS(s, t) DelayNonBlock(s, (t) * (UNIT_US))
#define DelayNonBlockMS(s, t) DelayNonBlock(s, (t) * (UNIT_MS))
#define DelayNonBlockS(s, t)  DelayNonBlock(s, (t) * (UNIT_S))

//---------------------------------------------------------------
//

void FirewareDelay(u32 nWaitTime);

#endif
