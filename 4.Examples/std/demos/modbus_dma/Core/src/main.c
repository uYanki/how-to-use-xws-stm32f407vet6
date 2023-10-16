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

void PulseGenInit(void);
void PulseGenN(uint32_t count);

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    eMBInit(MB_RTU, 0x01, 1, 115200, MB_PAR_EVEN);

    // const UCHAR ucSlaveID[] = {0xAA, 0xBB, 0xCC};
    // eMBSetSlaveID(0x33, FALSE, ucSlaveID, 3);

    eMBEnable();

    // PulseGenConfig(10000, 230);
    PulseGenInit();

    WaveGenInit();

    P(DrvCfg).u16BlinkPeriod = 1000;

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
            PulseGenN(8);
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
