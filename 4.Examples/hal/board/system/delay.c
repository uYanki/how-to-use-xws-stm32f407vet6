#include "delay.h"

#define DWT_FREQ         (168e6)  // 168MHz

#define DWT_CR           *(volatile uint32_t*)0xE0001000
#define DWT_CYCCNT       *(volatile uint32_t*)0xE0001004
#define DEM_CR           *(volatile uint32_t*)0xE000EDFC

#define DEM_CR_TRCENA    (1 << 24)
#define DWT_CR_CYCCNTENA (1 << 0)

void DelayInit(void)
{
    // enable dwt
    DEM_CR |= (uint32_t)DEM_CR_TRCENA;

    // clear cyccnt
    DWT_CYCCNT = (uint32_t)0u;

    // enable cyccnt
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;
}

void DelayBlock(uint32_t us)
{
    tick_t ticks, tCnt = 0;
    tick_t tPrev, tCur;

    ticks = us * (DWT_FREQ / 1000000);
    tPrev = DWT_CYCCNT;

    while (tCnt < ticks)
    {
        if ((tCur = DWT_CYCCNT) != tPrev)
        {
            if (tCur > tPrev)
            {
                tCnt += tCur - tPrev;
            }
            else
            {
                tCnt += UINT32_MAX + tCur - tPrev;
            }
            tPrev = tCur;
        }
    }
}

tick_t DelayGetTick(void)
{
    // don't use DelayGetTick() overload HAL_GetTick()
    // it may be cause function execution to fail,eg: SystemClock_Config(),
    // HAL_CAN_AddTxMessage()...
    return DWT_CYCCNT;
}
