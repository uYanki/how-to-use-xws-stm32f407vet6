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

bool PulseCalc(u32 clk, u32 freq, u16* prd, u16* psc, u32* frq)
{
    if (freq > clk)
    {
        return false;
    }

    u16 _prd = 0;
    u16 _psc = 0;
    u32 _frq = 0;

    for (u32 prescaler = 1; prescaler <= 65536; ++prescaler)
    {
        f32 period = (f32)clk / (f32)freq / (f32)prescaler;

        if (((u32)period < 2) || ((u32)period > 65536))
        {
            continue;
        }

        if ((period - (f32)(u32)period) > 0.5f)
        {
            continue;
        }

        // �Ĵ���ֵ

        *prd = period - 1;
        *psc = prescaler - 1;

        // ʵ��Ƶ��

        *frq = clk / (*prd + 1) / (*psc + 1);

        // Ƶ�����

        if (*frq == freq)
        {
            return true;
        }

        // Ƶ�����

        if (abs(freq - *frq) < (freq * 0.05))
        {
            if (abs(freq - *frq) < (abs(freq) - _frq))
            {
                _prd = *prd;
                _psc = *psc;
                _frq = frq;
            }
        }
    }

    if (_frq != 0)
    {
        *prd = _prd;
        *psc = _psc;
        return true;
    }

    return false;
}

// range of duty: (0,1000)
bool PulseConfig(u32 freq, u16 duty)
{
    TIM_TypeDef* TIMx = TIM13;

    /**
     * @note ʱ��Ƶ��
     *
     *      - APB1_GPIO_CLK: 42M
     *      - APB1_TIM_CLK: 84M
     *
     * @note ʱ�ӷ�Ƶ
     *
     *      - TIM_Prescaler: clksrc = internal (APBx)
     *      - TIM_ClockDivision: clksrc = external, �����벶��, �൱�˲�
     *
     * @note PWM ģʽ
     *
     *    ���ϼ���ʱ:
     *
     *      - TIM_OCMode = TIM_OCMode_PWM1:
     *
     *          TIMx_CNT < TIMx_CCRn, �����Ч��ƽ
     *          TIMx_CNT > TIMx_CCRn, �����Ч��ƽ
     *
     *      - TIM_OCMode = TIM_OCMode_PWM2:
     *
     *          TIMx_CNT < TIMx_CCRn, �����Ч��ƽ
     *          TIMx_CNT > TIMx_CCRn, �����Ч��ƽ
     *
     * @note ��Ч��ƽ
     *
     *    - TIM_OCPolarity = TIM_OCPolarity_High���ߵ�ƽ��
     *    - TIM_OCPolarity = TIM_OCPolarity_Low ���͵�ƽ��
     *
     */

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    u32 freq_o;

    if (!PulseCalc(84e6, freq,
                   (u16*)&TIM_TimeBaseStructure.TIM_Period,
                   (u16*)&TIM_TimeBaseStructure.TIM_Prescaler, &freq_o))
    {
        TIM_Cmd(TIMx, DISABLE);

        return false;
    }

    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_Pulse       = (TIM_TimeBaseStructure.TIM_Period + 1) * 0.666f;  // ռ�ձ�
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIMx, &TIM_OCInitStructure);

    // TIM_SelectOnePulseMode(TIMx, TIM_OPMode_Single);  // one pulse mode
    TIM_SelectOnePulseMode(TIMx, TIM_OPMode_Repetitive);  // pwm mode

    TIM_Cmd(TIMx, ENABLE);

    return true;
}
