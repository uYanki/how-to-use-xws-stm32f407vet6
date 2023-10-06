#include "gen.h"

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

bool PulseCalc(u32 clk, u32 freq, f32 duty, u16* prd, u16* psc, u16* ccr)
{
    if (freq > clk)
    {
        return false;
    }

    if (duty <= 0.f || duty >= 1.f)
    {
        return false;
    }

    u32 prescaler;

    for (prescaler = 1; prescaler <= 65536; ++prescaler)
    {
        f32 period = (f32)clk / (f32)freq / (f32)prescaler;

        if (((u32)period == 0) || ((u32)period > 65536))
        {
            continue;
        }

        if ((period - (u32)period) > 0.5f)
        {
            continue;
        }

        // 寄存器值

        *ccr = period * duty;

        if (*ccr == 0)
        {
            continue;
        }

        *prd = period - 1;
        *psc = prescaler - 1;
        *ccr -= 1;

        // 实际频率

        u32 frq = clk / (*prd + 1) / (*psc + 1);

        // 频率相等

        if (frq == freq)
        {
            return true;
        }
    }

    return false;
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

    if (!PulseCalc(84e6, freq, duty / 1000.f,
                   (u16*)&TIM_TimeBaseStructure.TIM_Period,
                   (u16*)&TIM_TimeBaseStructure.TIM_Prescaler,
                   (u16*)&TIM_OCInitStructure.TIM_Pulse))
    {
        TIM_Cmd(TIMx, DISABLE);

        return false;
    }

    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;

    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
    TIM_OC1Init(TIMx, &TIM_OCInitStructure);

    // TIM_SelectOnePulseMode(TIMx, TIM_OPMode_Single);  // one pulse mode
    TIM_SelectOnePulseMode(TIMx, TIM_OPMode_Repetitive);  // pwm mode

    TIM_Cmd(TIMx, ENABLE);

    return true;
}
