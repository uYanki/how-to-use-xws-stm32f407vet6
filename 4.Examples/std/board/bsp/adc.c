#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"

#include "system/autoinit.h"

#include "adc.h"
#include "paratbl/tbl.h"

//-----------------------------------------------------------------------------
//

//-------------------------------------
// adc

#define ADC_CLKEN       RCC_APB2PeriphClockCmd
#define ADC_CLK         RCC_APB2Periph_ADC1
#define ADC_PORT        ADC1
#define ADC_MAX_RANK    5  // 通道数

//-------------------------------------
// channel

// 模拟量输入 (Analog Input): ADC12_IN8
#define AI1_GPIO_CLK    RCC_AHB1Periph_GPIOB
#define AI1_GPIO_PORT   GPIOB
#define AI1_GPIO_PIN    GPIO_Pin_0
#define AI1_ADC_CH      ADC_Channel_8
#define AI1_ADC_RANK    1

// 模拟量输入 (Analog Input): ADC12_IN9
#define AI2_GPIO_CLK    RCC_AHB1Periph_GPIOB
#define AI2_GPIO_PORT   GPIOB
#define AI2_GPIO_PIN    GPIO_Pin_1
#define AI2_ADC_CH      ADC_Channel_9
#define AI2_ADC_RANK    2

// 芯片温度 (Temperature Sensor): ADC1_IN16
#define TS_ADC_CH       ADC_Channel_16
#define TS_ADC_RANK     3

// 参考电压 (Reference Voltage): ADC1_IN17
#define VREF_ADC_CH     ADC_Channel_17
#define VREF_ADC_RANK   4

// 电池电压 (Battery Voltage): ADC1_IN18
#define VBAT_ADC_CH     ADC_Channel_18
#define VBAT_ADC_RANK   5

//-------------------------------------
// dma, over-sample

#define ADC_DMA_CLKEN   RCC_AHB1PeriphClockCmd
#define ADC_DMA_CLK     RCC_AHB1Periph_DMA2
#define ADC_DMA_CHANNEL DMA_Channel_0
#define ADC_DMA_STREAM  DMA2_Stream0

#define ADC_OVER_SAMPLE 2  // 过采样位数

#if ((1 <= ADC_OVER_SAMPLE) && (ADC_OVER_SAMPLE <= 4))

#define ADC_OVS_SW

#define ADC_DMA_FLAG       DMA_FLAG_TCIF0
#define ADC_DMA_IT         DMA_IT_TCIF0
#define ADC_DMA_IRQn       DMA2_Stream0_IRQn
#define ADC_DMA_IRQHandler DMA2_Stream0_IRQHandler

#define ADC_DMA_BUF_SIZE   ARRAY_SIZE(su16AdcOVS)
#define ADC_DMA_BUF_ADDR   su16AdcOVS

static u16 su16AdcOVS[ADC_MAX_RANK * (1 << ADC_OVER_SAMPLE)] = {0};

#else

#define ADC_DMA_BUF_SIZE ADC_MAX_RANK
#define ADC_DMA_BUF_ADDR P_ADDR(DbgSta.u16DbgBuf[0])

#endif

//

static int BspAdcInit(void)
{
    // GPIO
    {
        GPIO_InitTypeDef GPIO_InitStructure = {
            .GPIO_Mode  = GPIO_Mode_AN,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        };

        RCC_AHB1PeriphClockCmd(AI1_GPIO_CLK, ENABLE);  // AI1
        GPIO_InitStructure.GPIO_Pin = AI1_GPIO_PIN;
        GPIO_Init(AI1_GPIO_PORT, &GPIO_InitStructure);
        GPIO_PinLockConfig(AI1_GPIO_PORT, AI1_GPIO_PIN);

        RCC_AHB1PeriphClockCmd(AI2_GPIO_CLK, ENABLE);  // AI2
        GPIO_InitStructure.GPIO_Pin = AI2_GPIO_PIN;
        GPIO_Init(AI2_GPIO_PORT, &GPIO_InitStructure);
        GPIO_PinLockConfig(AI2_GPIO_PORT, AI2_GPIO_PIN);
    }

    // DMA
    {
        ADC_DMA_CLKEN(ADC_DMA_CLK, ENABLE);

        DMA_InitTypeDef DMA_InitStructure = {
            .DMA_Channel            = ADC_DMA_CHANNEL,
            .DMA_PeripheralBaseAddr = (u32) & (ADC_PORT->DR),
            .DMA_Memory0BaseAddr    = (u32)ADC_DMA_BUF_ADDR,
            .DMA_DIR                = DMA_DIR_PeripheralToMemory,
            .DMA_BufferSize         = ADC_DMA_BUF_SIZE,
            .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
            .DMA_MemoryInc          = DMA_MemoryInc_Enable,
            .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
            .DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord,
            .DMA_Mode               = DMA_Mode_Circular,
            .DMA_Priority           = DMA_Priority_High,
            .DMA_FIFOMode           = DMA_FIFOMode_Disable,
            .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
            .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
        };

        DMA_Init(ADC_DMA_STREAM, &DMA_InitStructure);
        DMA_Cmd(ADC_DMA_STREAM, ENABLE);
    }
#ifdef ADC_OVS_SW
    // nvic
    {
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel                   = ADC_DMA_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        // enable irq
        DMA_ClearITPendingBit(ADC_DMA_STREAM, ADC_DMA_IT);
        DMA_ITConfig(ADC_DMA_STREAM, DMA_IT_TC, ENABLE);
    }
#endif
    // ADC
    {
        ADC_CLKEN(ADC_CLK, ENABLE);

        ADC_CommonInitTypeDef ADC_CommonInitStructure = {
            .ADC_Mode             = ADC_Mode_Independent,
            .ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles,  // 通道间采样间隔
            .ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled,
            .ADC_Prescaler        = ADC_Prescaler_Div4,
        };

        ADC_InitTypeDef ADC_InitStructure = {
            .ADC_Resolution           = ADC_Resolution_12b,
            .ADC_ScanConvMode         = ENABLE,
            .ADC_ContinuousConvMode   = ENABLE,
            .ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None,
            .ADC_DataAlign            = ADC_DataAlign_Right,
            .ADC_NbrOfConversion      = ADC_MAX_RANK,
        };

        ADC_CommonInit(&ADC_CommonInitStructure);
        ADC_Init(ADC_PORT, &ADC_InitStructure);

        // wake up temperature sensor
        ADC_TempSensorVrefintCmd(ENABLE);

        // 配置规则组
#if ADC_OVER_SAMPLE == 4
        // in 16-bit oversampling mode, a shorter sampling time can also achieve higher accuracy
        ADC_RegularChannelConfig(ADC_PORT, AI1_ADC_CH, AI1_ADC_RANK, ADC_SampleTime_15Cycles);
        ADC_RegularChannelConfig(ADC_PORT, AI2_ADC_CH, AI2_ADC_RANK, ADC_SampleTime_15Cycles);
        ADC_RegularChannelConfig(ADC_PORT, TS_ADC_CH, TS_ADC_RANK, ADC_SampleTime_15Cycles);
        ADC_RegularChannelConfig(ADC_PORT, VREF_ADC_CH, VREF_ADC_RANK, ADC_SampleTime_15Cycles);
        ADC_RegularChannelConfig(ADC_PORT, VBAT_ADC_CH, VBAT_ADC_RANK, ADC_SampleTime_15Cycles);
#else
        ADC_RegularChannelConfig(ADC_PORT, AI1_ADC_CH, AI1_ADC_RANK, ADC_SampleTime_56Cycles);
        ADC_RegularChannelConfig(ADC_PORT, AI2_ADC_CH, AI2_ADC_RANK, ADC_SampleTime_56Cycles);
        ADC_RegularChannelConfig(ADC_PORT, TS_ADC_CH, TS_ADC_RANK, ADC_SampleTime_56Cycles);
        ADC_RegularChannelConfig(ADC_PORT, VREF_ADC_CH, VREF_ADC_RANK, ADC_SampleTime_56Cycles);
        ADC_RegularChannelConfig(ADC_PORT, VBAT_ADC_CH, VBAT_ADC_RANK, ADC_SampleTime_56Cycles);
#endif

        // Enable DMA request after last transfer (Single-ADC mode)
        ADC_DMARequestAfterLastTransferCmd(ADC_PORT, ENABLE);

        ADC_DMACmd(ADC_PORT, ENABLE);
        ADC_Cmd(ADC_PORT, ENABLE);
    }

    // start conv
    ADC_SoftwareStartConv(ADC_PORT);

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(BspAdcInit, INIT_LEVEL_DEVICE)

#ifdef ADC_OVS_SW

void ADC_DMA_IRQHandler(void)
{
    if (DMA_GetITStatus(ADC_DMA_STREAM, ADC_DMA_IT) == SET)
    {
        // clear flag
        DMA_ClearITPendingBit(ADC_DMA_STREAM, ADC_DMA_IT);
        DMA_ClearFlag(ADC_DMA_STREAM, ADC_DMA_FLAG);

        // calc average

        // clang-format off
        u16  u16Cnt = 1 << ADC_OVER_SAMPLE;
        u16* pu16DatSrc = ADC_DMA_BUF_ADDR;
        u32  au32DatDst[ADC_MAX_RANK] = {0};
        // clang-format on

        while (u16Cnt--)
        {
            // 由于内核有多个加法器, 因此这种写法相当于并行运算
            // (参考自 arm 的 dsp 库)
            au32DatDst[0] += *pu16DatSrc++;
            au32DatDst[1] += *pu16DatSrc++;
            au32DatDst[2] += *pu16DatSrc++;
            au32DatDst[3] += *pu16DatSrc++;
            au32DatDst[4] += *pu16DatSrc++;
        }

        P(DbgSta.u16DbgBuf[0]) = au32DatDst[0] >> ADC_OVER_SAMPLE;
        P(DbgSta.u16DbgBuf[1]) = au32DatDst[1] >> ADC_OVER_SAMPLE;
        P(DbgSta.u16DbgBuf[2]) = au32DatDst[2] >> ADC_OVER_SAMPLE;
        P(DbgSta.u16DbgBuf[3]) = au32DatDst[3] >> ADC_OVER_SAMPLE;
        P(DbgSta.u16DbgBuf[4]) = au32DatDst[4] >> ADC_OVER_SAMPLE;

#if 1
        static tick_t t        = 0;
        P(DbgSta.u16DbgBuf[7]) = dwt_tick() - t;
        t                      = dwt_tick();
#endif

#if 0
        Vsense = Value×3300/4096  （mV）
        Temp = （（Vsense - V25）/Avg_Slope）+25
        V25是Vsense 在25度时候的值，Avg_Slope 是温度转换率。这两个值可在datasheet里边查到。
        V25 = 0.76V   Avg_Slope = 2.5 mV/°C
        Temp = (Vsense– 760)/2500 + 25;
#endif

        f32 Vsense             = P(DbgSta.u16DbgBuf[2]) / 4096.f * 3300;  // mV
        f32 temp               = (Vsense - 760) / 2500.f + 25;
        P(DbgSta.u16DbgBuf[8]) = temp * 100;  // 0.01 °
    }
}

#endif
