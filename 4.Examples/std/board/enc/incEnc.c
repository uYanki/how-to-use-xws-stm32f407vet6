
#include "system/sleep.h"
#include "paratbl/tbl.h"

#include "incEnc.h"

//----------------------------------------------------------
// ports

#define ENC_TIM_CLK       RCC_APB1Periph_TIM4
#define ENC_TIM_PORT      TIM4
#define ENC_TIM_CH_A      TIM_Channel_1
#define ENC_TIM_CH_B      TIM_Channel_2
#define ENC_TIM_MAX_PRD   (0x7FFF)

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

#define TIM_BaseLine      ((ENC_TIM_MAX_PRD) >> 1)

static inline s32 IncEncGet(void)
{
    return (s32)TIM_GetCounter(ENC_TIM_PORT) - (s32)TIM_BaseLine;
}

static inline void IncEncRst(void)
{
    TIM_SetCounter(ENC_TIM_PORT, TIM_BaseLine);
}

/**
 * [in]  u32EncRes
 */
void IncEncInit(IncEncArgs_t* args)
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
    }
    // tim
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

        RCC_APB1PeriphClockCmd(ENC_TIM_CLK, ENABLE);

        TIM_TimeBaseStructure.TIM_Prescaler         = 0;  // No prescaling
        TIM_TimeBaseStructure.TIM_Period            = 4 * (*args->u32EncRes) - 1;
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
    IncEncRst();

    args->u32LastTick = 0;

    TIM_Cmd(ENC_TIM_PORT, ENABLE);
}

/**
 * [in]  u16SpdCoeff
 * [in]  u32EncRes
 * [out] s32EncPos
 * [out] s32EncTurns
 * [out] s32UserSpdFb
 * [out] s64UserPosFb
 */
s32 IncEncCycle(IncEncArgs_t* args)
{
    u32 u32CurTick = dwt_tick();

    if (args->u32LastTick == 0)
    {
        args->u32LastTick = u32CurTick;
        return 0;
    }

    RO s32 s32EncRes   = (s32)(*args->u32EncRes);
    s32    s32DeltaPos = IncEncGet();
    IncEncRst();  // immediately reset, otherwise pulse loss may occur

    // speed (rpm)
    {
        // deltaTime_s = (f32)HAL_DeltaTick(args->u32LastTick, u32CurTick) / (f32)SystemCoreClock
        // speed_rpm = 60 * (f32)s32DeltaPos / (f32)s32EncRes / (f32)deltaTime_s
        // speed_out = u16SpdCoeff * speed_rpm

        s32 s32Tmp;

        s32Tmp = 60 * (s32)(*args->u16SpdCoeff);
        s32Tmp = s32Tmp * (s32)s32DeltaPos * (s32)SystemCoreClock;
        s32Tmp = s32Tmp / (s32)s32EncRes / (s32)HAL_DeltaTick(args->u32LastTick, u32CurTick);

        *args->s32UserSpdFb = s32Tmp;
    }

    // position (pulse)
    {
        *args->s64UserPosFb += (s64)s32DeltaPos;
        *args->u32EncPos   = (s64)(*args->s64UserPosFb) % (s64)s32EncRes;
        *args->s32EncTurns = (s64)(*args->s64UserPosFb) / (s64)s32EncRes;
    }

    // time (microsecond,us)
    args->u32LastTick = u32CurTick;

    return 0;
}
