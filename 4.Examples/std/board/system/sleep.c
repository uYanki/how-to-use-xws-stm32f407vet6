#include "sleep.h"

//---------------------------------------------------------------
//

u32 GetSysClk(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    return clocks.SYSCLK_Frequency;
}

//---------------------------------------------------------------
//

#define DWT_FREQ         (SystemCoreClock) // GetSysClk()

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

u32 dwt_tick(void)
{
    return DWT_CYCCNT;
}

//---------------------------------------------------------------
//

#define TIM_CLK  RCC_APB1Periph_TIM3
#define TIM_PORT TIM3

void tim_init(void)  // freq = SystemCoreClock/2 Hz
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(TIM_CLK, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler     = (SystemCoreClock / 1e6) - 1;
    TIM_TimeBaseStructure.TIM_Period        = 2 - 1;  // TIMx->ARR
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Down;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM_PORT, &TIM_TimeBaseStructure);
}

void _tim_wait(u16 us)
{
    TIM_PORT->CNT = us - 1;
    TIM_PORT->CR1 |= TIM_CR1_CEN;
    while ((TIM_PORT->SR & TIM_FLAG_Update) != SET)
        ;
    TIM_PORT->CR1 &= (~TIM_CR1_CEN);
    TIM_PORT->SR &= ~TIM_FLAG_Update;
}

void tim_wait(u32 us)
{
    u16 hi, lo;

    us >>= 1;

    hi = us >> 16;
    lo = us & 0xFFFF;

    while (hi--)
    {
        _tim_wait(0xFFFF);
    }

    _tim_wait(lo);
}
