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
#include "pulse/enc.h"

#include "sensor/ds18b20/ds18b20.h"

#define CONFIG_MODULE_WAVEGEN     1  // 波形发生器模块
#define CONFIG_MODULE_TEMPERATURE 1  // 环境温度采集模块
#define CONFIG_MODULE_ENCODER     1  // 编码器测速模块

//-----------------------------------------------------------------------------
//

static void TempSensorCycle(void);
static void EncoderCycle(void);

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

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    eMBInit(MB_RTU, 0x01, 1, 115200, MB_PAR_EVEN);

    // const UCHAR ucSlaveID[] = {0xAA, 0xBB, 0xCC};
    // eMBSetSlaveID(0x33, FALSE, ucSlaveID, 3);

    eMBEnable();

    PulseGenInit();
    PulseGenConfig(10000, 230);

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

#if CONFIG_MODULE_TEMPERATURE
        TempSensorCycle();
#endif
#if CONFIG_MODULE_ENCODER
        EncoderCycle();
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

//-----------------------------------------------------------------------------
// encoder: position recording, speed calculation

#if CONFIG_MODULE_ENCODER

static int EncoderInit(void)
{
    EncInit();

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(EncoderInit, INIT_LEVEL_BOARD)

static void EncoderCycle(void)
{
    static tick_t t = 0;

    if (DelayNonBlockMS(t, 1))
    {
        t = HAL_GetTick();

        EncCycle();
    }
}

#endif

//-----------------------------------------------------------------------------
