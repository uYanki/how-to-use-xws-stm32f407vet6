#include "main.h"
#include "system/sleep.h"
#include "system/autoinit.h"
#include "bsp/uart.h"

#include "pinctrl.h"
#include "mbslave.h"

#include "pinmap.h"
#include "paratbl/tbl.h"

#include "pulse/pulse.h"
#include "pulse/wave.h"

#include "enc/incEnc.h"
#include "enc/pulse.h"

#include "sensor/ds18b20/ds18b20.h"

#define CONFIG_MODULE_WAVEGEN     1  // 波形发生器模块
#define CONFIG_MODULE_TEMPERATURE 1  // 环境温度采集模块
#define CONFIG_MODULE_ENCODER     1  // 编码器测速模块

//-----------------------------------------------------------------------------
//

static void BspTempSensorCycle(void);
static void BspEncCycle(void);

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

/***定时器1主模式***/
void TIM1_config(u32 Cycle)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;  // TIM1_CH4 PA11
    // GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_TIM1);

    TIM_TimeBaseStructure.TIM_Period            = Cycle - 1;
    TIM_TimeBaseStructure.TIM_Prescaler         = 84 - 1;              // 设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;        // 设置时钟分割：TDTS= Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;  // TIM向上计数模式
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                   // 重复计数，一定要=0！！！
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;         // 选择定时器模式：TIM脉冲宽度调制模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  // 比较输出使能
    TIM_OCInitStructure.TIM_Pulse       = Cycle / 2 - 1;           // 设置待装入捕获寄存器的脉冲值
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low;      // 输出极性

    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);
    TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);

    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
}
/***定时器2从模式***/
void TIM2_config(u32 PulseNum)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructure.TIM_Period        = PulseNum - 1;
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_SelectInputTrigger(TIM2, TIM_TS_ITR0);
    // TIM_InternalClockConfig(TIM3);
    TIM2->SMCR |= 0x07;  // 设置从模式寄存器
    // TIM_ITRxExternalClockConfig(TIM2, TIM_TS_ITR0);

    // TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
void Pulse_output(u32 Cycle, u32 PulseNum)
{
    TIM2_config(PulseNum);
    TIM_Cmd(TIM2, ENABLE);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM1_config(Cycle);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 高级定时器一定要加上，主输出使能
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  // TIM_IT_CC1
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除中断标志位
        TIM_CtrlPWMOutputs(TIM1, DISABLE);           // 主输出使能
        TIM_Cmd(TIM1, DISABLE);                      // 关闭定时器
        TIM_Cmd(TIM2, DISABLE);                      // 关闭定时器
        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
    }
}

//

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    eMBInit(MB_RTU, 0x01, 1, 115200, MB_PAR_EVEN);

    // const UCHAR ucSlaveID[] = {0xAA, 0xBB, 0xCC};
    // eMBSetSlaveID(0x33, FALSE, ucSlaveID, 3);

    eMBEnable();

    // PulseGenInit();
    // PulseGenConfig(10000, 230);

    WaveGenInit();

    Pulse_output(1000, 8000);  // 1KHZ，8000个脉冲

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

#if CONFIG_MODULE_TEMPERATURE
        BspTempSensorCycle();
#endif
#if CONFIG_MODULE_ENCODER
        BspEncCycle();
#endif
    }
}

//-----------------------------------------------------------------------------
// temperature sensor

#if CONFIG_MODULE_TEMPERATURE

static int BspTempSensorInit(void)
{
    ds18b20_init();
    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(BspTempSensorInit, INIT_LEVEL_BOARD)

static void BspTempSensorCycle(void)
{
    static tick_t t = 0;

    if (DelayNonBlockS(t, 1))
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

        t = HAL_GetTick();
    }
}

#endif

//-----------------------------------------------------------------------------
// encoder: position recording, speed calculation

#define CONFIG_ENC_PPS 60

static IncEncArgs_t* pEncArgs;

static int BspEncInit(void)
{
    static IncEncArgs_t EncArgs;

    EncArgs.u32EncRes    = (vu32*)P_ADDR(DrvCfg.u32EncRes);
    EncArgs.s32EncPos    = (vs32*)P_ADDR(PosCtl.s32EncPos);
    EncArgs.s32EncTurns  = (vs32*)P_ADDR(PosCtl.s32EncTurns);
    EncArgs.s16UserSpdFb = (vs16*)P_ADDR(MotSta.s16UserSpdFb);
    EncArgs.s64UserPosFb = (vs64*)P_ADDR(MotSta.s64UserPosFb);

    pEncArgs = &EncArgs;

    P(DrvCfg.u32EncRes) = CONFIG_ENC_PPS;

    IncEncInit(pEncArgs);
    PulseGenInit();

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(BspEncInit, INIT_LEVEL_BOARD)

static void BspEncCycle(void)
{
    static tick_t t = 0;

    if (DelayNonBlockMS(t, 1))
    {
        t = HAL_GetTick();

        IncEncCycle(pEncArgs);
    }
}

//-----------------------------------------------------------------------------

static int BspFreqDivOutInit(void)
{
    // PulseGenInit();
    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(BspFreqDivOutInit, INIT_LEVEL_BOARD)

static void BspFreqDivOutCycle(void)
{
    static tick_t t = 0;

    if (DelayNonBlockMS(t, 1))
    {
        t = HAL_GetTick();

        static s64 s64LastUserPosFb = 0;

        // s64 s64CurUserPosFb = P(MotSta).s64UserPosFb;

        // PulseGenConfig(abs(s64CurUserPosFb - s64LastUserPosFb), 500);
    }
}

//

#if 0

static PID_t SpdPID = {
    .Kp  = 0.1f,
    .Ki  = 0.2f,
    .Kd  = 0.0f,
    .out = 0,
    .lo  = 0,
    .hi  = 2000,
};

SpdPID.fbk = P(MotSta).s16UserSpdFb;
// SpdPID.ref = R(07, u16ServoOn) ? P(MotSta).s16UserSpdRef : 0;
SpdPID.ref = R(07, u16ServoOn) ? 2000 : 0;
SpdPID.Ts  = 1;  // pEncArgs->f32DeltaTick;

PID_Handler_Base(&SpdPID);

if (SpdPID.out > 2000)
{
    SpdPID.out = 2000;
}

R(07, s16VdRef) = SpdPID.out;
R(07, s16VqRef) = SpdPID.out;

#endif
