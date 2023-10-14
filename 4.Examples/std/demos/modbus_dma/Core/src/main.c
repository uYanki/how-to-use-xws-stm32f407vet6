#include "main.h"
#include "system/sleep.h"
#include "system/autoinit.h"
#include "bsp/uart.h"

#include "pinctrl.h"
#include "mbslave.h"

#include "pinmap.h"
#include "paratbl/tbl.h"

#include "pulse/gen.h"
#include "pulse/wave.h"

#include "sensor/ds18b20/ds18b20.h"

#define CONFIG_MODULE_TEMPERATURE 1  // 温度模块

//-----------------------------------------------------------------------------
//

static void TempSensorCycle(void);

//-----------------------------------------------------------------------------
//

void usdk_preinit(void)
{
    // sleep
    sleep_init();
    // delay
    DelayInit();
    // hw_uart
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    UART_Config(&USART_InitStructure);
    UART_DMA_Config();
}

//

//

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    eMBInit(MB_RTU, 0x01, 1, 115200, MB_PAR_EVEN);

    // const UCHAR ucSlaveID[] = {0xAA, 0xBB, 0xCC};
    // eMBSetSlaveID(0x33, FALSE, ucSlaveID, 3);

    eMBEnable();

    PulseInit();
    PulseConfig(434e2, 230);

    WaveGenInit();

    P(DrvCfg).u16BlinkPeriod = 500;

    while (1)
    {
        eMBPoll();

        //------------------------------------------------

        WriteBit(ucDiscInBuf[0], 0, !PEin(10));  // key1
        WriteBit(ucDiscInBuf[0], 1, !PEin(11));  // key2
        WriteBit(ucDiscInBuf[0], 2, !PEin(12));  // key3

        PEout(13) = !GETBIT(ucCoilBuf[0], 0);  // led1
        PEout(14) = !GETBIT(ucCoilBuf[0], 1);  // led2

#if 0
        PEout(15) = !GETBIT(ucCoilBuf[0], 2);  // led3
#else
        static tick_t t_blink = 0;
        if (DelayNonBlockMS(t_blink, P(DrvCfg).u16BlinkPeriod))
        {
            PEout(15) ^= 1;  // toggle
            t_blink = HAL_GetTick();
        }
#endif

        //------------------------------------------------

        WaveGenCycle();

        //------------------------------------------------

#if CONFIG_MODULE_TEMPERATURE
        TempSensorCycle();
#endif
    }
}

//-----------------------------------------------------------------------------
// temperature sensor

#if CONFIG_MODULE_TEMPERATURE

static int TempSensorInit(void)
{
    ds18b20_init();
    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(TempSensorInit, INIT_LEVEL_BOARD)

static void TempSensorCycle(void)
{
    static tick_t t_temp = 0;

    if (DelayNonBlockS(t_temp, 1))
    {
        s16 s16EnvTemp;

        __disable_irq();

        if (ds18b20_read_temp(&s16EnvTemp))
        {
            P(MotSta).s16EnvTemp = ds18b20_convert_temp(s16EnvTemp) * 100;
        }
        else
        {
            P(MotSta).s16EnvTemp = 0;
        }

        __enable_irq();

        t_temp = HAL_GetTick();
    }
}

#endif

/*****************************************
引脚说明：
PB6

TIM4_CH1(TIM4 -- APB1 16位  84MHZ)

*****************************************/

int Pwm_PB6_InputInit(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;
    TIM_ICInitTypeDef       TIM4_ICInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   // 时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  // 使能GPIOB时钟

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;         // GPIOB6
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;       // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;   // 下拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);              // 初始化

    // 定时器复用引脚
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);

    TIM_TimeBaseStructure.TIM_Prescaler     = 0;                   // 定时器分频
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseStructure.TIM_Period        = 50000 - 1;           // 自动重装载值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;        // 分频因子 配置死区时会用到

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    // 初始化TIM2输入捕获参数
    TIM4_ICInitStructure.TIM_Channel     = TIM_Channel_1;             // 通道1
    TIM4_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity_Rising;     // 上升沿捕获
    TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;  // 映射到TI1上
    TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;            // 配置输入分频,不分频
    TIM4_ICInitStructure.TIM_ICFilter    = 0x05;                      // IC3F=0000 配置输入滤波器 不滤波
    // 初始化 PWM 输入模式
    TIM_PWMIConfig(TIM4, &TIM4_ICInitStructure);

    // 当工作做 PWM 输入模式时,只需要设置触发信号的那一路即可（用于测量周期）
    // 另外一路（用于测量占空比）会由硬件自带设置，不需要再配置

    // 选择输入捕获的触发信号
    TIM_SelectInputTrigger(TIM4, TIM_TS_TI1FP1);

    // 选择从模式: 复位模式
    // PWM 输入模式时,从模式必须工作在复位模式，当捕获开始时,计数器 CNT 会被复位
    TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);
    TIM_SelectMasterSlaveMode(TIM4, TIM_MasterSlaveMode_Enable);

    // 使能捕获中断,这个中断针对的是主捕获通道（测量周期那个）
    TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;       // 抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;       // 子优先级0
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;  // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                                 // 根据指定的参数初始化VIC寄存器、

    // TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC2,ENABLE);				//允许更新中断 ,允许CC2IE捕获中断

    TIM_Cmd(TIM4, ENABLE);  // 使能定时器4

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    return 0;
}

USDK_INIT_EXPORT(Pwm_PB6_InputInit, INIT_LEVEL_COMPONENT)

u32   IC1Value, IC2Value;
float DutyCycle, Frequency;

// 定时器4中断服务程序
void TIM4_IRQHandler(void)
{
    /* 清除定时器捕获/比较 1 中断 */
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);

    /* 获取输入捕获值 */
    IC1Value = TIM_GetCapture1(TIM4);
    IC2Value = TIM_GetCapture2(TIM4);
    // printf("IC1Value = %d IC2Value = %d ",IC1Value,IC2Value);
    //  注意：捕获寄存器 CCR1 和 CCR2 的值在计算占空比和频率的时候必须加 1
    if (IC1Value != 0)
    {
        /* 占空比计算 */
        DutyCycle = (float)((IC2Value + 1) * 100) / (IC1Value + 1);

        /* 频率计算 */
        Frequency = 84000000 / (float)(IC1Value + 1);

        P(MotSta).u32DbgBuf[0] = DutyCycle;
        P(MotSta).u32DbgBuf[1] = Frequency;
    }
}
