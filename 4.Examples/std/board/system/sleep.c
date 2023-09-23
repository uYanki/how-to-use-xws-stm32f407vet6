#include "sleep.h"

//---------------------------------------------------------------
//

#define DWT_FREQ         (SystemCoreClock)  // GetSysClk()

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

//---------------------------------------------------------------
//

static vu32 m_ticks = 0;

void DelayInit(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / UNIT_MS);  // 1ms
}

void DelayBlock(u32 nWaitTime)
{
    u32 nStartTick = HAL_GetTick();

    while ((HAL_GetTick() - nStartTick) < nWaitTime)
        ;
}

uint32_t tickstart = 0;

bool DelayNonBlock(u32 nStartTick, u32 nWaitTime)
{
    return HAL_GetTick() >= (nStartTick + nWaitTime);
}

u32 HAL_GetTick(void)
{
    return m_ticks;
}

/**
 * @note call HAL_IncTick() in SysTick_Handler()
 *
 *      extern void HAL_IncTick(void);
 *      HAL_IncTick();
 *
 */
__weak void HAL_IncTick(void)
{
    m_ticks += UNIT_MS;
}

//---------------------------------------------------------------
//

void FirewareDelay(u32 nWaitTime)
{
    u8 n;
    while (nWaitTime--)
    {
        n = UINT8_MAX;
        while (n--)
        {
        }
    }
}
