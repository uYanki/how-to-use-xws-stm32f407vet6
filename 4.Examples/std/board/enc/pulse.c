#include "pulse/calc.h"
#include "pulse.h"

// #define TIM_FRQ            168000000ul  // 168M (APB2)

#define PULSE_GEN_MST_GPIO_CLKEN     RCC_AHB1PeriphClockCmd
#define PULSE_GEN_MST_GPIO_CLK       RCC_AHB1Periph_GPIOA
#define PULSE_GEN_MST_GPIO_PORT      GPIOA
#define PULSE_GEN_MST_GPIO_PIN       GPIO_Pin_3
#define PULSE_GEN_MST_GPIO_PINSRC    GPIO_PinSource3
#define PULSE_GEN_MST_GPIO_AF        GPIO_AF_TIM2

#define PULSE_GEN_MST_TIM_FRQ        84000000ul  // 84M (APB1)
#define PULSE_GEN_MST_TIM_CLKEN      RCC_APB1PeriphClockCmd
#define PULSE_GEN_MST_TIM_CLK        RCC_APB1Periph_TIM2
#define PULSE_GEN_MST_TIM_PORT       TIM2
#define PULSE_GEN_MST_TIM_CH         TIM_Channel_4
#define PULSE_GEN_MST_TIM_OCInit     TIM_OC4Init

#define PULSE_GEN_SLV_TIM_FRQ        84000000ul  // 84M (APB1)
#define PULSE_GEN_SLV_TIM_CLKEN      RCC_APB1PeriphClockCmd
#define PULSE_GEN_SLV_TIM_CLK        RCC_APB1Periph_TIM3
#define PULSE_GEN_SLV_TIM_PORT       TIM3
#define PULSE_GEN_SLV_TIM_ITR        TIM_TS_ITR1
#define PULSE_GEN_SLV_TIM_IRQn       TIM3_IRQn
#define PULSE_GEN_SLV_TIM_IRQHandler TIM3_IRQHandler
#define PULSE_GEN_SLV_TIM_TRGSRC     TIM_TRGOSource_OC1Ref  //??
#define PULSE_GEN_SLV_TIM_FLAG       TIM_FLAG_CC1
#define PULSE_GEN_SLV_TIM_IT         TIM_IT_CC1

// #define PULSE_GEN_SLV_TIM_TRGSRC     TIM_TRGOSource_OC4Ref  //??
// #define PULSE_GEN_SLV_TIM_FLAG       TIM_FLAG_CC4
// #define PULSE_GEN_SLV_TIM_IT         TIM_IT_CC4

//

void PULSE_GEN_SLV_TIM_IRQHandler()  // TIM3达到指定脉冲数回调函数
{
    if (SET == TIM_GetITStatus(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT))
    {
        TIM_CtrlPWMOutputs(PULSE_GEN_MST_TIM_PORT, DISABLE);

        TIM_ITConfig(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT, DISABLE);
        TIM_ClearITPendingBit(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT);

        TIM_Cmd(PULSE_GEN_MST_TIM_PORT, DISABLE);
        TIM_Cmd(PULSE_GEN_SLV_TIM_PORT, DISABLE);
    }
}

void PulseGenN(u32 count)  // 产生 PWM
{
    TIM_SetAutoreload(PULSE_GEN_MST_TIM_PORT, count - 1);
    TIM_ITConfig(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT, ENABLE);
    TIM_CtrlPWMOutputs(PULSE_GEN_MST_TIM_PORT, ENABLE);

    TIM_Cmd(PULSE_GEN_MST_TIM_PORT, ENABLE);
    TIM_Cmd(PULSE_GEN_SLV_TIM_PORT, ENABLE);
}

void PulseGenInit(void)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure = {
            .GPIO_Pin   = PULSE_GEN_MST_GPIO_PIN,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        };

        PULSE_GEN_MST_GPIO_CLKEN(PULSE_GEN_MST_GPIO_CLK, ENABLE);
        GPIO_PinAFConfig(PULSE_GEN_MST_GPIO_PORT, PULSE_GEN_MST_GPIO_PINSRC, PULSE_GEN_MST_GPIO_AF);
        GPIO_Init(PULSE_GEN_MST_GPIO_PORT, &GPIO_InitStructure);
    }
    // mst tim
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        TIM_OCInitTypeDef       TIM_OCInitStructure;

        // clkin
        PULSE_GEN_MST_TIM_CLKEN(PULSE_GEN_MST_TIM_CLK, ENABLE);
        TIM_InternalClockConfig(PULSE_GEN_MST_TIM_PORT);

        // timer
        TIM_TimeBaseStructure.TIM_Prescaler         = 84 - 1;
        TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period            = 100 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(PULSE_GEN_MST_TIM_PORT, &TIM_TimeBaseStructure);
        TIM_SetAutoreload(PULSE_GEN_MST_TIM_PORT, 0);

        // event
        TIM_SelectMasterSlaveMode(PULSE_GEN_MST_TIM_PORT, TIM_MasterSlaveMode_Enable);
        TIM_SelectOutputTrigger(PULSE_GEN_MST_TIM_PORT, TIM_TRGOSource_Update);

        // pwm
        TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_Pulse       = (TIM_TimeBaseStructure.TIM_Period + 1) * 0.33 - 1;  // duty
        TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        PULSE_GEN_MST_TIM_OCInit(PULSE_GEN_MST_TIM_PORT, &TIM_OCInitStructure);
        TIM_OC4FastConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCFast_Disable);
    }
    // slv tim
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

        // clkin
        PULSE_GEN_SLV_TIM_CLKEN(PULSE_GEN_SLV_TIM_CLK, ENABLE);

        // timer
        TIM_TimeBaseStructure.TIM_Prescaler         = 0;
        TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period            = 1024 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(PULSE_GEN_SLV_TIM_PORT, &TIM_TimeBaseStructure);
        TIM_SetAutoreload(PULSE_GEN_SLV_TIM_PORT, 0);

        // event
        TIM_SelectSlaveMode(PULSE_GEN_SLV_TIM_PORT, TIM_SlaveMode_External1);
        TIM_SelectInputTrigger(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_ITR);
        TIM_SelectMasterSlaveMode(PULSE_GEN_SLV_TIM_PORT, TIM_MasterSlaveMode_Disable);
        TIM_SelectOutputTrigger(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_TRGSRC);

        // nvic
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel                   = PULSE_GEN_SLV_TIM_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
}

#if 0


// 占空比: 0 ~ 1000 -> 0 ~ 100 %
bool PulseGenConfig(u32 freq, u16 duty)
{
    /**
     * @note 时钟频率
     *
     *      - APB1_GPIO_CLK: 42M
     *      - APB1_TIM_CLK: 84M
     *      - APB2_GPIO_CLK: 84M
     *      - APB2_TIM_CLK: 168M
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

    if (TIM_Calc(TIM_FRQ, freq, TIM_FRQ * 0.02,
                 (u16*)&TIM_TimeBaseStructure.TIM_Period,
                 (u16*)&TIM_TimeBaseStructure.TIM_Prescaler, nullptr) == false)
    {
        TIM_Cmd(PULSE_GEN_TIM_PORT, DISABLE);

        return freq == 0;
    }

    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(PULSE_GEN_TIM_PORT, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_Pulse       = (TIM_TimeBaseStructure.TIM_Period + 1) * (f32)duty / 1000 - 1;  // 占空比
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    PULSE_GEN_TIM_OCxInit(PULSE_GEN_TIM_PORT, &TIM_OCInitStructure);

    TIM_SelectOnePulseMode(PULSE_GEN_TIM_PORT, TIM_OPMode_Single);  // one pulse mode
    // TIM_SelectOnePulseMode(PULSE_GEN_TIM_PORT, TIM_OPMode_Repetitive);  // pwm mode

    TIM_Cmd(PULSE_GEN_TIM_PORT, ENABLE);

    return true;
}

#endif

//
