#include "pulse.h"
#include "calc.h"

#if 0

#define TIM_FRQ               84000000ul  // 84M

// generate

#define PULSE_GEN_GPIO_CLKEN  RCC_AHB1PeriphClockCmd
#define PULSE_GEN_GPIO_CLK    RCC_AHB1Periph_GPIOB
#define PULSE_GEN_GPIO_PORT   GPIOB
#define PULSE_GEN_GPIO_PIN    GPIO_Pin_0
#define PULSE_GEN_GPIO_PINSRC GPIO_PinSource0
#define PULSE_GEN_GPIO_AF     GPIO_AF_TIM3

#define PULSE_GEN_TIM_CLKEN   RCC_APB1PeriphClockCmd
#define PULSE_GEN_TIM_CLK     RCC_APB1Periph_TIM3
#define PULSE_GEN_TIM_PORT    TIM3
#define PULSE_GEN_TIM_CH      TIM_Channel_3
#define PULSE_GEN_TIM_OCxInit TIM_OC3Init

// capture

#define PULSE_CAP_GPIO_CLKEN  RCC_AHB1PeriphClockCmd
#define PULSE_CAP_GPIO_CLK    RCC_AHB1Periph_GPIOB
#define PULSE_CAP_GPIO_PORT   GPIOB
#define PULSE_CAP_GPIO_PIN    GPIO_Pin_1
#define PULSE_CAP_GPIO_PINSRC GPIO_PinSource1
#define PULSE_CAP_GPIO_AF     GPIO_AF_TIM3

#define PULSE_CAP_TIM_CLKEN   RCC_APB1PeriphClockCmd
#define PULSE_CAP_TIM_CLK     RCC_APB1Periph_TIM3
#define PULSE_CAP_TIM_PORT    TIM3
#define PULSE_CAP_TIM_CH      TIM_Channel_4

#else

#define TIM_FRQ                  168000000ul  // 168M

// generate

#define PULSE_GEN_GPIO_CLKEN     RCC_AHB1PeriphClockCmd
#define PULSE_GEN_GPIO_CLK       RCC_AHB1Periph_GPIOE
#define PULSE_GEN_GPIO_PORT      GPIOE
#define PULSE_GEN_GPIO_PIN       GPIO_Pin_5
#define PULSE_GEN_GPIO_PINSRC    GPIO_PinSource5
#define PULSE_GEN_GPIO_AF        GPIO_AF_TIM9

#define PULSE_GEN_TIM_CLKEN      RCC_APB2PeriphClockCmd
#define PULSE_GEN_TIM_CLK        RCC_APB2Periph_TIM9
#define PULSE_GEN_TIM_PORT       TIM9
#define PULSE_GEN_TIM_CH         TIM_Channel_1
#define PULSE_GEN_TIM_OCxInit    TIM_OC1Init

// capture

#define PULSE_CAP_GPIO_CLKEN     RCC_AHB1PeriphClockCmd
#define PULSE_CAP_GPIO_CLK       RCC_AHB1Periph_GPIOE
#define PULSE_CAP_GPIO_PORT      GPIOE
#define PULSE_CAP_GPIO_PIN       GPIO_Pin_6
#define PULSE_CAP_GPIO_PINSRC    GPIO_PinSource6
#define PULSE_CAP_GPIO_AF        GPIO_AF_TIM9

#define PULSE_CAP_TIM_CLKEN      RCC_APB2PeriphClockCmd
#define PULSE_CAP_TIM_CLK        RCC_APB2Periph_TIM9
#define PULSE_CAP_TIM_PORT       TIM9
#define PULSE_CAP_TIM_CH         TIM_Channel_2

#define PULSE_CAP_TIM_IRQn       TIM9_IRQn
#define PULSE_CAP_TIM_IRQHandler TIM9_IRQHandler

#endif

//

void PulseGenInit(void)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure = {
            .GPIO_Pin   = PULSE_GEN_GPIO_PIN,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        };

        PULSE_GEN_GPIO_CLKEN(PULSE_GEN_GPIO_CLK, ENABLE);
        GPIO_PinAFConfig(PULSE_GEN_GPIO_PORT, PULSE_GEN_GPIO_PINSRC, PULSE_GEN_GPIO_AF);
        GPIO_Init(PULSE_GEN_GPIO_PORT, &GPIO_InitStructure);
    }
    // tim
    {
        PULSE_GEN_TIM_CLKEN(PULSE_GEN_TIM_CLK, ENABLE);
    }
}

void PulseCapInit(void)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure = {
            .GPIO_Pin   = PULSE_CAP_GPIO_PIN,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        };

        PULSE_CAP_GPIO_CLKEN(PULSE_CAP_GPIO_CLK, ENABLE);
        GPIO_PinAFConfig(PULSE_CAP_GPIO_PORT, PULSE_CAP_GPIO_PINSRC, PULSE_CAP_GPIO_AF);
        GPIO_Init(PULSE_CAP_GPIO_PORT, &GPIO_InitStructure);
    }
    // tim
    {
        PULSE_CAP_TIM_CLKEN(PULSE_CAP_TIM_CLK, ENABLE);
    }
}

int Pwm_PB6_InputInit(void)
{
    // TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    // NVIC_InitTypeDef        NVIC_InitStructure;
    // TIM_ICInitTypeDef       TIM_ICInitStructure;

    // TIM_TimeBaseStructure.TIM_Prescaler     = 0;                   // 定时器分频
    // TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  // 向上计数模式
    // TIM_TimeBaseStructure.TIM_Period        = 50000 - 1;           // 自动重装载值
    // TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;        // 分频因子 配置死区时会用到

    // TIM_TimeBaseInit(PULSE_CAP_TIM_PORT, &TIM_TimeBaseStructure);

    // // 初始化TIM2输入捕获参数
    // TIM_ICInitStructure.TIM_Channel     = TIM_Channel_1;             // 通道1
    // TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;     // 上升沿捕获
    // TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  // 映射到TI1上
    // TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            // 配置输入分频,不分频
    // TIM_ICInitStructure.TIM_ICFilter    = 0x05;                      // IC3F=0000 配置输入滤波器 不滤波
    // // 初始化 PWM 输入模式
    // TIM_PWMIConfig(PULSE_CAP_TIM_PORT, &TIM_ICInitStructure);

    // // 当工作做 PWM 输入模式时,只需要设置触发信号的那一路即可（用于测量周期）
    // // 另外一路（用于测量占空比）会由硬件自带设置，不需要再配置

    // // 选择输入捕获的触发信号
    // TIM_SelectInputTrigger(PULSE_CAP_TIM_PORT, TIM_TS_TI1FP1);

    // // 选择从模式: 复位模式
    // // PWM 输入模式时,从模式必须工作在复位模式，当捕获开始时,计数器 CNT 会被复位
    // TIM_SelectSlaveMode(PULSE_CAP_TIM_PORT, TIM_SlaveMode_Reset);
    // TIM_SelectMasterSlaveMode(PULSE_CAP_TIM_PORT, TIM_MasterSlaveMode_Enable);

    // // 使能捕获中断,这个中断针对的是主捕获通道（测量周期那个）
    // TIM_ITConfig(PULSE_CAP_TIM_PORT, TIM_IT_CC1, ENABLE);
    // TIM_ClearITPendingBit(PULSE_CAP_TIM_PORT, TIM_IT_CC1);

    // NVIC_InitStructure.NVIC_IRQChannel                   = PULSE_CAP_TIM_PORT_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;       // 抢占优先级2
    // NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;       // 子优先级0
    // NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;  // IRQ通道使能
    // NVIC_Init(&NVIC_InitStructure);                                 // 根据指定的参数初始化VIC寄存器、

    // // TIM_ITConfig(PULSE_CAP_TIM_PORT,TIM_IT_Update|TIM_IT_CC2,ENABLE);				//允许更新中断 ,允许CC2IE捕获中断

    // TIM_Cmd(PULSE_CAP_TIM_PORT, ENABLE);  // 使能定时器4

    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    return 0;
}

// range of duty: (0,1000)
bool PulseGenConfig(u32 freq, u16 duty)
{
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

    if (TIM_Calc(TIM_FRQ, freq, TIM_FRQ * 0.02,
                 (u16*)&TIM_TimeBaseStructure.TIM_Period,
                 (u16*)&TIM_TimeBaseStructure.TIM_Prescaler, &freq_o) == false)
    {
        TIM_Cmd(PULSE_GEN_TIM_PORT, DISABLE);

        return false;
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

    // TIM_SelectOnePulseMode(TIMx, TIM_OPMode_Single);  // one pulse mode
    TIM_SelectOnePulseMode(PULSE_GEN_TIM_PORT, TIM_OPMode_Repetitive);  // pwm mode

    TIM_Cmd(PULSE_GEN_TIM_PORT, ENABLE);

    return true;
}

#include "paratbl/tbl.h"

void PULSE_CAP_TIM_IRQHandler(void)
{
    /* 清除定时器捕获/比较 1 中断 */
    TIM_ClearITPendingBit(PULSE_CAP_TIM_PORT, TIM_IT_CC1);

    /* 获取输入捕获值 */
    u32 IC1Value = TIM_GetCapture1(PULSE_CAP_TIM_PORT);
    u32 IC2Value = TIM_GetCapture2(PULSE_CAP_TIM_PORT);
    // printf("IC1Value = %d IC2Value = %d ",IC1Value,IC2Value);
    //  注意：捕获寄存器 CCR1 和 CCR2 的值在计算占空比和频率的时候必须加 1

    if (IC1Value != 0)
    {
        /* 占空比计算 */
        float DutyCycle = (float)((IC2Value + 1) * 100) / (IC1Value + 1);

        /* 频率计算 */
        float Frequency = 84000000 / (float)(IC1Value + 1);

        P(DbgSta).u32DbgBuf[0] = DutyCycle;
        P(DbgSta).u32DbgBuf[1] = Frequency;
    }
}
