
#include "system/sleep.h"
#include "paratbl/tbl.h"

#include "enc.h"

//----------------------------------------------------------
// config

#define CONFIG_ENC_PPR    (60)  // 分辨率

//----------------------------------------------------------
// ports

// TIMX = 1、2、3、4、5、8
#define ENC_TIM_CLK       RCC_APB1Periph_TIM4
#define ENC_TIM_PORT      TIM4
#define ENC_TIM_CH_A      TIM_Channel_1
#define ENC_TIM_CH_B      TIM_Channel_2
#define ENC_TIM_MAX_PRD   0x7FFFF

#define ENC_GPIO_AF       GPIO_AF_TIM4

#define ENC_A_GPIO_CLK    RCC_AHB1Periph_GPIOD
#define ENC_A_GPIO_PORT   GPIOD
#define ENC_A_GPIO_PIN    GPIO_Pin_12
#define ENC_A_GPIO_PINSRC GPIO_PinSource12

#define ENC_B_GPIO_CLK    RCC_AHB1Periph_GPIOD
#define ENC_B_GPIO_PORT   GPIOD
#define ENC_B_GPIO_PIN    GPIO_Pin_13
#define ENC_B_GPIO_PINSRC GPIO_PinSource13

//----------------------------------------------------------
// funcs

#define TIM_BaseLine      ((ENC_TIM_MAX_PRD) >> 4)

static inline s32 EncGet(void)
{
    return (s32)TIM_GetCounter(ENC_TIM_PORT) - (s32)TIM_BaseLine;
}

static inline void EncRst(void)
{
    TIM_SetCounter(ENC_TIM_PORT, TIM_BaseLine);
}

void EncInit(void)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(ENC_A_GPIO_CLK | ENC_B_GPIO_CLK, ENABLE);

        GPIO_PinAFConfig(ENC_A_GPIO_PORT, ENC_A_GPIO_PINSRC, ENC_GPIO_AF);
        GPIO_PinAFConfig(ENC_B_GPIO_PORT, ENC_B_GPIO_PINSRC, ENC_GPIO_AF);

        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

        GPIO_InitStructure.GPIO_Pin = ENC_A_GPIO_PIN;
        GPIO_Init(ENC_A_GPIO_PORT, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = ENC_B_GPIO_PIN;
        GPIO_Init(ENC_B_GPIO_PORT, &GPIO_InitStructure);

        // RCC_AHB1PeriphClockCmd(ENC_Z_GPIO_CLK, ENABLE);
        // GPIO_InitStructure.GPIO_Pin = ENC_Z_GPIO_PIN;
        // GPIO_Init(ENC_Z_GPIO_PORT, &GPIO_InitStructure);
    }
    // tim
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

        RCC_APB1PeriphClockCmd(ENC_TIM_CLK, ENABLE);

        TIM_TimeBaseStructure.TIM_Prescaler         = 0;  // No prescaling
        TIM_TimeBaseStructure.TIM_Period            = (4 * CONFIG_ENC_PPR) - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

        TIM_TimeBaseInit(ENC_TIM_PORT, &TIM_TimeBaseStructure);
    }
    // capture
    {
        TIM_ICInitTypeDef TIM_ICInitStructure;

        TIM_ICStructInit(&TIM_ICInitStructure);
        TIM_ICInitStructure.TIM_ICFilter = 2;  // 0x0 ~ 0xF
        TIM_ICInitStructure.TIM_Channel  = ENC_TIM_CH_A;
        TIM_ICInit(ENC_TIM_PORT, &TIM_ICInitStructure);
        TIM_ICInitStructure.TIM_Channel = ENC_TIM_CH_B;
        TIM_ICInit(ENC_TIM_PORT, &TIM_ICInitStructure);

        TIM_EncoderInterfaceConfig(ENC_TIM_PORT, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    }

    // Reset counter
    EncRst();

    P(DrvCfg).u32EncRes = CONFIG_ENC_PPR;

    TIM_Cmd(ENC_TIM_PORT, ENABLE);
}

void EncCycle(void)
{
    static u32 u32LastTick = 0;

    if (u32LastTick != 0)
    {
        s32 s32EncDeltaPos = EncGet();
        EncRst();  // 立即复位计数器

        u32 u32DeltaTick = HAL_DeltaTick(u32LastTick, dwt_tick());

        P(PosCtl).s32EncPos += s32EncDeltaPos;

        if (P(PosCtl).s32EncPos > (s32)P(DrvCfg).u32EncRes)
        {
            P(PosCtl).s32EncPos -= (s32)P(DrvCfg).u32EncRes;
        }
        else if (P(PosCtl).s32EncPos < 0)
        {
            P(PosCtl).s32EncPos += (s32)P(DrvCfg).u32EncRes;
        }

        // speed (rpm)
        P(MotSta).s16UserSpdFb = 60 * s32EncDeltaPos / ((f64)u32DeltaTick / (f64)SystemCoreClock) / (s32)P(DrvCfg).u32EncRes;

        // position (pulse)
        P(MotSta).s64UserPosFb += (s64)s32EncDeltaPos;
        P(PosCtl).s32EncTurns = P(MotSta).s64UserPosFb / (s64)P(DrvCfg).u32EncRes;
    }

    u32LastTick = dwt_tick();
}
