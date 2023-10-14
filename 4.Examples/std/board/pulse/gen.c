#include "gen.h"

#define TIM_FRQ 84e6

void PulseInit(void)  // PA6 (TIM13_CH1)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM13);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// err_i: maximum allowable error
bool PulseCalc(u32 clk_i, u32 frq_i, u32 err_i, u16* prd_o, u16* psc_o, u32* frq_o)
{
    if (frq_i == 0)
    {
        return false;
    }

#if 1
    if (frq_i > clk_i)
#else
    // because period >= 2
    if (frq_i > (clk_i >> 1))
#endif
    {
        return false;
    }

    bool similar = false;
    u32  prd, psc, frq;
    u32  err_pre = err_i, err_cur;

    // for (prd = 2; prd <= 65536u; ++prd)
    for (prd = 65536u; prd >= 2; --prd)
    {
        psc = (f32)clk_i / (f32)frq_i / (f32)prd;

        if (psc == 0 || psc > 65536)
        {
            continue;
        }

        // 实际频率

        frq = (f32)clk_i / (f32)psc / (f32)prd;

        if (frq > clk_i)
        {
            continue;
        }

        // 频率误差

        if (frq > frq_i)
        {
            err_cur = frq - frq_i;
        }
        else
        {
            err_cur = frq_i - frq;
        }

        if (err_pre > err_cur)
        {
            err_pre = err_cur;

            *frq_o = frq;
            *prd_o = prd - 1;
            *psc_o = psc - 1;

            // 频率相等

            if (err_cur == 0)
            {
                return true;
            }

            // 频率相近

            similar = true;
        }
    }

    return similar;
}

// range of duty: (0,1000)
bool PulseConfig(u32 freq, u16 duty)
{
    TIM_TypeDef* TIMx = TIM13;

    /**
     * @note 时钟频率
     *
     *      - APB1_GPIO_CLK: 42M
     *      - APB1_TIM_CLK: 84M
     *
     * @note 时钟分频
     *
     *      - TIM_Prescaler: clksrc = internal (APBx)
     *      - TIM_ClockDivision: clksrc = external, 如输入捕获, 相当滤波
     *
     * @note PWM 模式
     *
     *    向上计数时:
     *
     *      - TIM_OCMode = TIM_OCMode_PWM1:
     *
     *          TIMx_CNT < TIMx_CCRn, 输出有效电平
     *          TIMx_CNT > TIMx_CCRn, 输出无效电平
     *
     *      - TIM_OCMode = TIM_OCMode_PWM2:
     *
     *          TIMx_CNT < TIMx_CCRn, 输出无效电平
     *          TIMx_CNT > TIMx_CCRn, 输出有效电平
     *
     * @note 有效电平
     *
     *    - TIM_OCPolarity = TIM_OCPolarity_High（高电平）
     *    - TIM_OCPolarity = TIM_OCPolarity_Low （低电平）
     *
     */

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    u32 freq_o;

    if (PulseCalc(TIM_FRQ, freq, TIM_FRQ * 0.02,
                  (u16*)&TIM_TimeBaseStructure.TIM_Period,
                  (u16*)&TIM_TimeBaseStructure.TIM_Prescaler, &freq_o) == false)
    {
        TIM_Cmd(TIMx, DISABLE);

        return false;
    }

    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_Pulse       = (TIM_TimeBaseStructure.TIM_Period + 1) * (f32)duty / 1000 - 1;  // 占空比
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIMx, &TIM_OCInitStructure);

    // TIM_SelectOnePulseMode(TIMx, TIM_OPMode_Single);  // one pulse mode
    TIM_SelectOnePulseMode(TIMx, TIM_OPMode_Repetitive);  // pwm mode

    TIM_Cmd(TIMx, ENABLE);

    return true;
}
