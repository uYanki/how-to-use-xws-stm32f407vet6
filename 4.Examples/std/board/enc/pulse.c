#include "pulse/calc.h"
#include "pulse.h"

//---------------------------------------------------------
// GPIO

#define PULSE_GEN_MST_GPIO_CLKEN     RCC_AHB1PeriphClockCmd
#define PULSE_GEN_MST_GPIO_CLK       RCC_AHB1Periph_GPIOA
#define PULSE_GEN_MST_GPIO_AF        GPIO_AF_TIM2

//---------------------------------------------------------
// Master TIM

#define PULSE_GEN_MST_TIM_FRQ        84000000ul  // 84M (APB1)
#define PULSE_GEN_MST_TIM_CLKEN      RCC_APB1PeriphClockCmd
#define PULSE_GEN_MST_TIM_CLK        RCC_APB1Periph_TIM2
#define PULSE_GEN_MST_TIM_PORT       TIM2

//---------------------------------------------------------
// Slave TIM

#define PULSE_GEN_SLV_TIM_FRQ        84000000ul  // 84M (APB1)
#define PULSE_GEN_SLV_TIM_CLKEN      RCC_APB1PeriphClockCmd
#define PULSE_GEN_SLV_TIM_CLK        RCC_APB1Periph_TIM3
#define PULSE_GEN_SLV_TIM_PORT       TIM3
#define PULSE_GEN_SLV_TIM_ITR        TIM_TS_ITR1
#define PULSE_GEN_SLV_TIM_IRQn       TIM3_IRQn
#define PULSE_GEN_SLV_TIM_IRQHandler TIM3_IRQHandler

//---------------------------------------------------------
// TIM Channel

#define PULSE_GEN_MST_TIM_CH         4  // <--- config

#if PULSE_GEN_MST_TIM_CH == 1  // CH1: PA0

#define PULSE_GEN_MST_GPIO_PORT   GPIOA
#define PULSE_GEN_MST_GPIO_PIN    GPIO_Pin_0
#define PULSE_GEN_MST_GPIO_PINSRC GPIO_PinSource0

#define PULSE_GEN_SLV_TIM_TRGSRC  TIM_TRGOSource_OC1Ref
#define PULSE_GEN_SLV_TIM_IT      TIM_IT_CC1

#elif PULSE_GEN_MST_TIM_CH == 4  // CH4: PA3

#define PULSE_GEN_MST_GPIO_PORT   GPIOA
#define PULSE_GEN_MST_GPIO_PIN    GPIO_Pin_3
#define PULSE_GEN_MST_GPIO_PINSRC GPIO_PinSource3

#define PULSE_GEN_SLV_TIM_TRGSRC  TIM_TRGOSource_OC4Ref
#define PULSE_GEN_SLV_TIM_IT      TIM_IT_CC4

#else

#error "please select a pulse generate channel"

#endif

//---------------------------------------------------------

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
 *      - TIM_ClockDivision: clksrc = external, 外部时钟滤波时间 (TIM1ETR..)
 *
 * @note 对齐模式
 *
 *      - TIM_CounterMode:
 *           边沿对齐模式: 计数器根据方向位递增计数或递减计数。
 *              TIM_CounterMode_Up:   0->TIM_Period
 *              TIM_CounterMode_Down: TIM_Period->0
 *           中心对齐模式: 计数器交替进行递增计数和递减计数
 *              TIM_CounterMode_CenterAligned1: 0->TIM_Period->0, 仅递减时,才输出比较中断  (TIM_IT_CC)
 *              TIM_CounterMode_CenterAligned2: 0->TIM_Period->0, 仅递减时,才输出比较中断
 *              TIM_CounterMode_CenterAligned3: 0->TIM_Period->0, 递减递增,都输出比较中断
 *           注:只要计数器处于使能状态, 就不得从边沿对产模式切换为中小对产模
 *
 * @note 重复计数
 *
 *      - TIM_RepetitionCounter: 在定时器重复完成n次计数后, 产生溢出中断, 即更新事件 (TIM_IT_Update)
 *           作用? 触发ADC采样或DAC输出...
 *
 * @note 有效电平
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
 * @note 触发源
 *
 *      Trigger Source: TI1FP1, TI1(Timer input 1) after FP(filter and polarity selection)
 *
 * @note 单脉冲模式 (One Pulse)
 *
 *     TIM_SelectOnePulseMode(PULSE_GEN_TIM_PORT, TIM_OPMode_Single);  // one pulse mode
 *     TIM_SelectOnePulseMode(PULSE_GEN_TIM_PORT, TIM_OPMode_Repetitive);  // pwm mode
 *
 * @note 自动重装载预装载允许位 (Auto-reload preload enable, ARPE)
 *
 *      Disable：自动重装载寄存器写入新值后，计数器立即产生计数溢出，然后开始新的计数周期
 *      Enable： 自动重装载寄存器写入新值后，计数器完成当前旧的计数后，再开始新的计数周期
 */

void PULSE_GEN_SLV_TIM_IRQHandler()  // 达到指定脉冲数
{
    if (SET == TIM_GetITStatus(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT))
    {
        TIM_ClearITPendingBit(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT);
        PulseGenStop();
    }
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
            .GPIO_PuPd  = GPIO_PuPd_UP,
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
        TIM_ARRPreloadConfig(PULSE_GEN_MST_TIM_PORT, DISABLE);  // disable ARPE

        // pwm
        TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_Pulse       = (TIM_TimeBaseStructure.TIM_Period + 1) * 0.33 - 1;  // duty
        TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

#if (PULSE_GEN_MST_TIM_CH == 1)
        TIM_OC1Init(PULSE_GEN_MST_TIM_PORT, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCPreload_Disable);
        TIM_OC1FastConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCFast_Disable);
#elif (PULSE_GEN_MST_TIM_CH == 4)
        TIM_OC4Init(PULSE_GEN_MST_TIM_PORT, &TIM_OCInitStructure);
        TIM_OC4PreloadConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCPreload_Disable);
        TIM_OC4FastConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCFast_Disable);
#endif

        // event
        TIM_SelectMasterSlaveMode(PULSE_GEN_MST_TIM_PORT, TIM_MasterSlaveMode_Enable);
        TIM_SelectOutputTrigger(PULSE_GEN_MST_TIM_PORT, PULSE_GEN_SLV_TIM_TRGSRC);
    }
    // slv tim
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

        // clkin
        PULSE_GEN_SLV_TIM_CLKEN(PULSE_GEN_SLV_TIM_CLK, ENABLE);

        // timer
        TIM_TimeBaseStructure.TIM_Prescaler         = 0;
        TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period            = 0;
        TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(PULSE_GEN_SLV_TIM_PORT, &TIM_TimeBaseStructure);

        // event
        TIM_SelectSlaveMode(PULSE_GEN_SLV_TIM_PORT, TIM_SlaveMode_External1);
        TIM_SelectInputTrigger(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_ITR);
        TIM_SelectMasterSlaveMode(PULSE_GEN_SLV_TIM_PORT, TIM_MasterSlaveMode_Disable);
        TIM_SelectOutputTrigger(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_TRGSRC);

        // nvic
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel                   = PULSE_GEN_SLV_TIM_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        TIM_ITConfig(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT, ENABLE);
    }
}

bool PulseGenConfig(u32 freq, f32 duty)  // 配置脉冲
{
#if 1

    if ((PULSE_GEN_MST_TIM_PORT->CR1 & TIM_CR1_CEN) ||
        (PULSE_GEN_SLV_TIM_PORT->CR1 & TIM_CR1_CEN))
    {
        // 配置失败: 脉冲正在生成
        return false;
    }

#endif

    if (freq == 0)
    {
        // 配置失败: 脉冲频率不能为零
        return false;
    }

    u16 prd, psc, ccr;

    if (TIM_Calc(PULSE_GEN_MST_TIM_FRQ, freq, freq * 0.02, &prd, &psc, nullptr) == false)
    {
        // 配置失败: 无法生成目标频率的脉冲
        return false;
    }

    ccr = (prd + 1) * duty;

    if (ccr == 0)
    {
        // 配置失败: 占空比不正确
        return false;
    }

    // 比较值
    ccr -= 1;

    // 频率
    PULSE_GEN_MST_TIM_PORT->PSC = psc;  // 分频
    PULSE_GEN_MST_TIM_PORT->ARR = prd;  // 周期

    // 占空比

#if PULSE_GEN_MST_TIM_CH == 1
    PULSE_GEN_MST_TIM_PORT->CCR1 = ccr;
#elif PULSE_GEN_MST_TIM_CH == 4
    PULSE_GEN_MST_TIM_PORT->CCR4 = ccr;
#endif

    // 配置成功
    return true;
}

bool PulseGenStart(u32 count)  // 生成脉冲
{
    if ((PULSE_GEN_MST_TIM_PORT->CR1 & TIM_CR1_CEN) ||
        (PULSE_GEN_SLV_TIM_PORT->CR1 & TIM_CR1_CEN))
    {
        // 配置失败: 脉冲正在生成
        return false;
    }

    if (count == 0 || count > 65536)
    {
        // 配置失败: 取值范围超过上下限制
        // TODO: 16为位定时器,待测单次最多个数时 65535 还是 65536
        return false;
    }

    --count;

    if (count == 0)
    {
        // 单脉冲模式
        TIM_SelectOnePulseMode(PULSE_GEN_MST_TIM_PORT, TIM_OPMode_Single);

        // enable pulse generator
        TIM_Cmd(PULSE_GEN_MST_TIM_PORT, ENABLE);
    }
    else
    {
        // PWM 模式
        TIM_SelectOnePulseMode(PULSE_GEN_MST_TIM_PORT, TIM_OPMode_Repetitive);

        // 脉冲生成个数 (必须大于2)
        TIM_SetAutoreload(PULSE_GEN_SLV_TIM_PORT, count);

        // enable pulse generator
        TIM_CtrlPWMOutputs(PULSE_GEN_MST_TIM_PORT, ENABLE);
        TIM_Cmd(PULSE_GEN_MST_TIM_PORT, ENABLE);
        TIM_Cmd(PULSE_GEN_SLV_TIM_PORT, ENABLE);
    }

    return true;
}

void PulseGenStop(void)
{
    // disable pulse generator
    TIM_Cmd(PULSE_GEN_MST_TIM_PORT, DISABLE);
    TIM_Cmd(PULSE_GEN_SLV_TIM_PORT, DISABLE);
    TIM_CtrlPWMOutputs(PULSE_GEN_MST_TIM_PORT, DISABLE);
}
