#include "adc.h"

#include "system/sleep.h"
#include "paratbl/tbl.h"

//------------------------------------------------------------------------------
// single ADC + DMA 16位过采样
//
// 附:
//  - F4 只能做软件过采样, H7 有硬件过采样
//  - F4 没有ADC校准函数, F1,H7 有ADC校准函数

//------------------------------------------------------------------------------
// port

typedef struct {
    u8 Channel;     // ADC_Channel_
    u8 SampleTime;  // ADC_SampleTime_
} ADConv_t;

#define CONFIG_ADC_OVS_RATIO  16u  // 过采样率
#define CONFIG_ADC_DMA_IRQ_SW 1u   // 使能DMA中断

// Channels
static RO ADConv_t aADConv[] = {
    {ADC_Channel_8,  ADC_SampleTime_3Cycles}, // AI1
    {ADC_Channel_9,  ADC_SampleTime_3Cycles}, // AI2
    {ADC_Channel_16, ADC_SampleTime_3Cycles}, // TS
    {ADC_Channel_17, ADC_SampleTime_3Cycles}, // VREF
    {ADC_Channel_18, ADC_SampleTime_3Cycles}, // VBAT
};

// Channel Index
#define CONFIG_INDEX_CHIP_TEMP 2
#define CONFIG_INDEX_VREF      3

//-------------------------------------
// GPIO

// 模拟量输入 (Analog Input): ADC12_IN8
#define AI1_GPIO_CLK           RCC_AHB1Periph_GPIOB
#define AI1_GPIO_PORT          GPIOB
#define AI1_GPIO_PIN           GPIO_Pin_0

// 模拟量输入 (Analog Input): ADC12_IN9
#define AI2_GPIO_CLK           RCC_AHB1Periph_GPIOB
#define AI2_GPIO_PORT          GPIOB
#define AI2_GPIO_PIN           GPIO_Pin_1

// 芯片温度 (Temperature Sensor): ADC1_IN16
// 参考电压 (Reference Voltage): ADC1_IN17
// 电池电压 (Battery Voltage): ADC1_IN18

//-------------------------------------
// ADC

#define ADC_CLKEN              RCC_APB2PeriphClockCmd
#define ADC_CLK                RCC_APB2Periph_ADC1
#define ADC_PORT               ADC1
#define ADC_RES                12u  // 分辨率

//-------------------------------------
// DMA

#define ADC_DMA_CLKEN          RCC_AHB1PeriphClockCmd
#define ADC_DMA_CLK            RCC_AHB1Periph_DMA2
#define ADC_DMA_CHANNEL        DMA_Channel_0
#define ADC_DMA_STREAM         DMA2_Stream0

#define ADC_DMA_FLAG           DMA_FLAG_TCIF0
#define ADC_DMA_IT             DMA_IT_TCIF0
#define ADC_DMA_IRQn           DMA2_Stream0_IRQn
#define ADC_DMA_IRQHandler     DMA2_Stream0_IRQHandler

//------------------------------------------------------------------------------
// Regular Oversampler

#if (CONFIG_ADC_OVS_RATIO < ADC_RES)
#define ADC_OVS_RSHIFT 0u
#else
#define ADC_OVS_RSHIFT (CONFIG_ADC_OVS_RATIO - ADC_RES)
#endif

#define ADC_CH_ADDR  aADConv
#define ADC_CH_NBR   ARRAY_SIZE(aADConv)

#define ADC_BUF_ADDR au16ADBuf
#define ADC_BUF_SIZE ARRAY_SIZE(au16ADBuf)

static u16 au16ADBuf[ADC_CH_NBR * (1 << ADC_OVS_RSHIFT)] = {0};

//------------------------------------------------------------------------------
// function

void AdcInit(void)
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
            .DMA_Memory0BaseAddr    = (u32)ADC_BUF_ADDR,
            .DMA_DIR                = DMA_DIR_PeripheralToMemory,
            .DMA_BufferSize         = ADC_BUF_SIZE,
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
#if CONFIG_ADC_DMA_IRQ_SW
    // NVIC
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
            .ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_8Cycles,  // 通道间采样间隔
            .ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled,
            .ADC_Prescaler        = ADC_Prescaler_Div4,
        };

        ADC_InitTypeDef ADC_InitStructure = {
            .ADC_Resolution           = ADC_Resolution_12b,
            .ADC_ScanConvMode         = ENABLE,
            .ADC_ContinuousConvMode   = ENABLE,
            .ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None,
            .ADC_DataAlign            = ADC_DataAlign_Right,
            .ADC_NbrOfConversion      = ADC_CH_NBR,
        };

        ADC_CommonInit(&ADC_CommonInitStructure);
        ADC_Init(ADC_PORT, &ADC_InitStructure);

        // wake up temperature sensor
        ADC_TempSensorVrefintCmd(ENABLE);

        // 配置规则组
        for (u8 u8ChIdx = 0; u8ChIdx < ADC_CH_NBR; ++u8ChIdx)
        {
#if 0 && (CONFIG_ADC_OVS_RATIO >= 16)
            // in 16-bit oversampling mode, a shorter sampling time can also achieve higher accuracy
            ADC_RegularChannelConfig(ADC_PORT, ADC_CH_ADDR[u8ChIdx].Channel, u8ChIdx + 1, ADC_SampleTime_3Cycles);
#else
            ADC_RegularChannelConfig(ADC_PORT, ADC_CH_ADDR[u8ChIdx].Channel, u8ChIdx + 1, ADC_CH_ADDR[u8ChIdx].SampleTime);
#endif
        }

        // Enable DMA request after last transfer (Single-ADC mode)
        ADC_DMARequestAfterLastTransferCmd(ADC_PORT, ENABLE);

        ADC_DMACmd(ADC_PORT, ENABLE);
        ADC_Cmd(ADC_PORT, ENABLE);
    }

    // start conv
    ADC_SoftwareStartConv(ADC_PORT);
}

// ARRAY_SIZE(pau16Output) = ADC_CH_NBR
void AdcOvsCalc(vu16* pau16Output)
{
    u16 u16Cnt;

    // 求和

    u16Cnt = 1 << ADC_OVS_RSHIFT;

    u16* pu16Input           = ADC_BUF_ADDR;
    u32  au32Sum[ADC_CH_NBR] = {0};

    while (u16Cnt--)
    {
        /**
         * 内核有4加法器, 此写法相当于并行运算 (参考自 arm 的 dsp 库)
         *
         * 同时在在不影响代码运行效率情况下, 适应了不同的通道数:
         *
         *    +---------------------+-------------------------+
         *    |  宏定义中无法使用    |   作为替代而使用          |
         *    |---------------------+-------------------------|
         *    |  #if ADC_CH_NBR > 1 |  if (ADC_CH_NBR > 1) {  |
         *    |  ...                |      ...                |
         *    |  #endif             |  }                      |
         *    +---------------------+-------------------------+
         *
         *   在编译后进入 debug 模式查看该段代码对应汇编。由于 ADC_CH_NBR 为常数, 该段代码
         *   的 `if 语句` 和 `不生效的 if 块` 会被编译器优化掉了，不会影响代码运行效率。
         *
         */

        // clang-format off
        if (ADC_CH_NBR >  0) { au32Sum[ 0] += *pu16Input++; }
        if (ADC_CH_NBR >  1) { au32Sum[ 1] += *pu16Input++; }
        if (ADC_CH_NBR >  2) { au32Sum[ 2] += *pu16Input++; }
        if (ADC_CH_NBR >  3) { au32Sum[ 3] += *pu16Input++; }
        if (ADC_CH_NBR >  4) { au32Sum[ 4] += *pu16Input++; }
        if (ADC_CH_NBR >  5) { au32Sum[ 5] += *pu16Input++; }
        if (ADC_CH_NBR >  6) { au32Sum[ 6] += *pu16Input++; }
        if (ADC_CH_NBR >  7) { au32Sum[ 7] += *pu16Input++; }
        if (ADC_CH_NBR >  8) { au32Sum[ 8] += *pu16Input++; }
        if (ADC_CH_NBR >  9) { au32Sum[ 9] += *pu16Input++; }
        if (ADC_CH_NBR > 10) { au32Sum[10] += *pu16Input++; }
        if (ADC_CH_NBR > 11) { au32Sum[11] += *pu16Input++; }
        if (ADC_CH_NBR > 12) { au32Sum[12] += *pu16Input++; }
        if (ADC_CH_NBR > 13) { au32Sum[13] += *pu16Input++; }
        if (ADC_CH_NBR > 14) { au32Sum[14] += *pu16Input++; }
        if (ADC_CH_NBR > 15) { au32Sum[15] += *pu16Input++; }
        if (ADC_CH_NBR > 16) { au32Sum[16] += *pu16Input++; }
        if (ADC_CH_NBR > 17) { au32Sum[17] += *pu16Input++; }
        if (ADC_CH_NBR > 18) { au32Sum[18] += *pu16Input++; }
        // clang-format on
    }

    // 求平均

    u16Cnt = ADC_CH_NBR;

    while (u16Cnt--)
    {
        pau16Output[u16Cnt] = au32Sum[u16Cnt] >> ADC_OVS_RSHIFT;
    }
}

#ifdef CONFIG_INDEX_CHIP_TEMP

// AD采样值转换为芯片温度
static u16 ConvToChipTemp(u16 advalue)  // 0.01 °C
{
    // Vsense = ADValue × 3300 / 4096 (mV)
    // Temp = ( (Vsense) - V25 ) / AvgSlope ) +25
    // 25° 时的 Vsense: V25 = 0.76V
    // 温度转换率 AvgSlope = 2.5 mV/°C

    return 2500 + (advalue * 3300 / 4096 - 760) / 25;
}

#endif

#if CONFIG_ADC_DMA_IRQ_SW

void ADC_DMA_IRQHandler(void)
{
    if (DMA_GetITStatus(ADC_DMA_STREAM, ADC_DMA_IT) == SET)
    {
        // clear flag
        DMA_ClearITPendingBit(ADC_DMA_STREAM, ADC_DMA_IT);
        DMA_ClearFlag(ADC_DMA_STREAM, ADC_DMA_FLAG);

        AdcOvsCalc((vu16*)P_ADDR(MotSta.u16AdcSampRes[0]));

        // measure time
        P(MotSta.u32AdcSampTime) = TimeRecEnd(TID_0);
        TimeRecStart(TID_0);  // 16-bit: 10ms

#ifdef CONFIG_INDEX_CHIP_TEMP
        P(MotSta).s16ChipTemp = ConvToChipTemp(P(MotSta.u16AdcSampRes[CONFIG_INDEX_CHIP_TEMP]));
#endif
#ifdef CONFIG_INDEX_VREF
#endif
    }
}

#endif
