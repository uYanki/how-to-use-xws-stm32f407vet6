#include "sleep.h"
#include "autoinit.h"

#define DWT_FREQ         (SystemCoreClock)

#define DWT_CR           *(vu32*)0xE0001000
#define DWT_CYCCNT       *(vu32*)0xE0001004
#define DEM_CR           *(vu32*)0xE000EDFC

#define DEM_CR_TRCENA    (1 << 24)
#define DWT_CR_CYCCNTENA (1 << 0)

// static int dwt_init(void)
void dwt_init(void)
{
    // enable dwt
    DEM_CR |= (u32)DEM_CR_TRCENA;

    // clear cyccnt
    DWT_CYCCNT = (u32)0u;

    // enable cyccnt
    DWT_CR |= (u32)DWT_CR_CYCCNTENA;
}

void dwt_wait(u32 us)
{
    u32 ticks, tcnt = 0;
    u32 t_old, t_now;

    ticks = us * (DWT_FREQ / 1000000);  // 节拍数
    t_old = DWT_CYCCNT;

    while (tcnt < ticks)
    {
        if ((t_now = DWT_CYCCNT) != t_old)
        {
            if (t_now > t_old)
            {
                tcnt += t_now - t_old;
            }
            else
            {
                tcnt += UINT32_MAX + t_now - t_old;
            }
            t_old = t_now;
        }
    }
}

u32 GetSysClk(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    return clocks.SYSCLK_Frequency;
}
