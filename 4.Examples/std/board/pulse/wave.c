#include "stm32f4xx.h"

#include "wave.h"

//-----------------------------------------------------------------------------
// ports

#define TIM_FRQ        84e6

#define DAC1_TIM_CLKEN RCC_APB1PeriphClockCmd
#define DAC1_TIM_CLK   RCC_APB1Periph_TIM6
#define DAC1_TIM_PORT  TIM6
#define DAC1_TRGO      DAC_Trigger_T6_TRGO

#define DAC2_TIM_CLKEN RCC_APB1PeriphClockCmd
#define DAC2_TIM_CLK   RCC_APB1Periph_TIM5
#define DAC2_TIM_PORT  TIM5
#define DAC2_TRGO      DAC_Trigger_T5_TRGO

//-----------------------------------------------------------------------------
// funcs

static void WaveGenDemo(void);
static bool WaveGenCfg(WaveGen_t* pWaveGen, u32 DAC_Channel_x);
extern bool PulseCalc(u32 clk_i, u32 frq_i, u32 err_i, u16* prd_o, u16* psc_o, u32* frq_o);

//-----------------------------------------------------------------------------
// wave

static RO u16 aSine12bit[] = {
    2047, 2189, 2331, 2472, 2611, 2747, 2879, 3008, 3131, 3250, 3362, 3468, 3568, 3660, 3744, 3819, 3886, 3944, 3993, 4033, 4062, 4082, 4092, 4092, 4082, 4062, 4033, 3993, 3944, 3886, 3819, 3744, 3660, 3568, 3468, 3362, 3250, 3131, 3008, 2879, 2747, 2611, 2472, 2331, 2189, 2047, 1905, 1763, 1622, 1483, 1347, 1215, 1086, 963, 844, 732, 626, 526, 434, 350, 275, 208, 150, 101, 61, 32, 12, 2, 2, 12, 32, 61, 101, 150, 208, 275, 350, 434, 526, 626, 732, 844, 963, 1086, 1215, 1347, 1483, 1622, 1763, 1905};

//-----------------------------------------------------------------------------
//

void WaveGenInit(void)
{
    // DAC_OUT1 = PA.4
    // DAC_OUT2 = PA.5

    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    // dac
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

        DAC_DeInit();
    }

    // tim
    {
        DAC1_TIM_CLKEN(DAC1_TIM_CLK, ENABLE);
        TIM_SelectOutputTrigger(DAC1_TIM_PORT, DAC1_TRGO);

        DAC2_TIM_CLKEN(DAC2_TIM_CLK, ENABLE);
        TIM_SelectOutputTrigger(DAC2_TIM_PORT, DAC2_TRGO);
    }

#if 1
    // demo
    WaveGenDemo();
#endif
}

void WaveGenCycle(void)
{
    switch ((WaveConfig_e)(P(WaveGen1).u16WaveConfig))
    {
        case WaveConfig_Doit:
            switch (WaveGenCfg(&P(WaveGen1), DAC_Channel_1))
            {
                case true: P(WaveGen1).u16WaveConfig = WaveConfig_Success; break;
                case false: P(WaveGen1).u16WaveConfig = WaveConfig_Failure; break;
            }
        default:
        case WaveConfig_Success:
        case WaveConfig_Failure:
            break;
    }

    switch ((WaveConfig_e)(P(WaveGen2).u16WaveConfig))
    {
        case WaveConfig_Doit:
            switch (WaveGenCfg(&P(WaveGen2), DAC_Channel_2))
            {
                case true: P(WaveGen2).u16WaveConfig = WaveConfig_Success; break;
                case false: P(WaveGen2).u16WaveConfig = WaveConfig_Failure; break;
            }
        default:
        case WaveConfig_Success:
        case WaveConfig_Failure:
            break;
    }
}

static bool WaveGenCfg(WaveGen_t* pWaveGen, u32 DAC_Channel_x)
{
    // DAC_Channel_1: DMA1_Stream5 channel7
    // DAC_Channel_2: DMA1_Stream6 channel7

    assert_param(IS_DAC_CHANNEL(DAC_Channel_x));

    //-----------------------------------------------------
    // ports

    uint32_t            DACx_TRGO;
    TIM_TypeDef*        DACx_TIM;
    DMA_Stream_TypeDef* DMA_Stream_x;

    if (DAC_Channel_1 == DAC_Channel_x)
    {
        DACx_TRGO    = DAC1_TRGO;
        DACx_TIM     = DAC1_TIM_PORT;
        DMA_Stream_x = DMA1_Stream5;
    }
    else  // DAC_Channel_2
    {
        DACx_TRGO    = DAC2_TRGO;
        DACx_TIM     = DAC2_TIM_PORT;
        DMA_Stream_x = DMA1_Stream6;
    }

    TIM_Cmd(DACx_TIM, DISABLE);  // stop TIM

    pWaveGen->u32WaveActualFreq = 0;

    //-----------------------------------------------------
    // DAC & DMA

    uint16_t u16WaveSize;

    switch ((WaveType_e)(pWaveGen->u16WaveType))
    {
        default: pWaveGen->u16WaveType = (u16)(WaveType_None);
        case WaveType_None: {
            // DAC
            DAC_InitTypeDef DAC_InitStructure = {
                DAC_InitStructure.DAC_Trigger        = DAC_Trigger_None,
                DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None,
                DAC_InitStructure.DAC_OutputBuffer   = DAC_OutputBuffer_Disable,
            };
            DAC_Init(DAC_Channel_x, &DAC_InitStructure);
            // DMA
            DMA_Cmd(DMA_Stream_x, DISABLE);
            return true;
        }
        case WaveType_Noise: {
            // DAC
            DAC_InitTypeDef DAC_InitStructure = {
                .DAC_Trigger                      = DACx_TRGO,
                .DAC_WaveGeneration               = DAC_WaveGeneration_Noise,
                .DAC_OutputBuffer                 = DAC_OutputBuffer_Disable,
                .DAC_LFSRUnmask_TriangleAmplitude = (pWaveGen->u16WaveAmplitude & 0xF) << 8,  // LFSRUnmask
            };
            DAC_Init(DAC_Channel_x, &DAC_InitStructure);
            // TIM
            u16WaveSize = 1;
            break;
        }
        case WaveType_Triangle: {
            // DAC
            DAC_InitTypeDef DAC_InitStructure = {
                .DAC_Trigger                      = DACx_TRGO,
                .DAC_WaveGeneration               = DAC_WaveGeneration_Triangle,
                .DAC_OutputBuffer                 = DAC_OutputBuffer_Disable,
                .DAC_LFSRUnmask_TriangleAmplitude = (pWaveGen->u16WaveAmplitude & 0xF) << 8,  // TriangleAmplitude
            };
            DAC_Init(DAC_Channel_x, &DAC_InitStructure);
            // TIM
            u16WaveSize = (2 << (pWaveGen->u16WaveAmplitude & 0xF)) * 2;
            break;
        }
        case WaveType_Sine: {
            // DAC
            {
                DAC_InitTypeDef DAC_InitStructure = {
                    .DAC_Trigger        = DACx_TRGO,
                    .DAC_WaveGeneration = DAC_WaveGeneration_None,
                    .DAC_OutputBuffer   = DAC_OutputBuffer_Disable,
                };
                DAC_Init(DAC_Channel_x, &DAC_InitStructure);
                DAC_DMACmd(DAC_Channel_x, ENABLE);
            }
            // DMA
            {
                DMA_InitTypeDef DMA_InitStructure = {
                    // base
                    .DMA_Channel            = DMA_Channel_7,
                    .DMA_DIR                = DMA_DIR_MemoryToPeripheral,
                    .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                    .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                    .DMA_Mode               = DMA_Mode_Circular,
                    .DMA_Priority           = DMA_Priority_High,
                    .DMA_FIFOMode           = DMA_FIFOMode_Disable,
                    .DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull,
                    .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                    .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
                    // data src
                    .DMA_Memory0BaseAddr    = (u32)&aSine12bit,
                    .DMA_BufferSize         = ARRAY_SIZE(aSine12bit),
                    .DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord,
                    // data dest
                    .DMA_PeripheralBaseAddr = (u32)(&DAC->DHR12R1 + (DAC_Channel_x >> 4) * 3),
                    .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
                };
                DMA_Cmd(DMA_Stream_x, DISABLE);
                DMA_DeInit(DMA_Stream_x);
                DMA_Init(DMA_Stream_x, &DMA_InitStructure);
                DMA_Cmd(DMA_Stream_x, ENABLE);
                // TIM
                u16WaveSize = DMA_InitStructure.DMA_BufferSize;
            }
            break;
        }
        case WaveType_Custom: {
            // DAC
            {
                DAC_InitTypeDef DAC_InitStructure = {
                    .DAC_Trigger        = DACx_TRGO,
                    .DAC_WaveGeneration = DAC_WaveGeneration_None,
                    .DAC_OutputBuffer   = DAC_OutputBuffer_Disable,
                };
                DAC_Init(DAC_Channel_x, &DAC_InitStructure);
                DAC_DMACmd(DAC_Channel_x, ENABLE);
            }
            // DMA
            {
                DMA_InitTypeDef DMA_InitStructure = {
                    .DMA_Channel         = DMA_Channel_7,
                    .DMA_DIR             = DMA_DIR_MemoryToPeripheral,
                    .DMA_PeripheralInc   = DMA_PeripheralInc_Disable,
                    .DMA_MemoryInc       = DMA_MemoryInc_Enable,
                    .DMA_Mode            = DMA_Mode_Circular,
                    .DMA_Priority        = DMA_Priority_High,
                    .DMA_FIFOMode        = DMA_FIFOMode_Disable,
                    .DMA_FIFOThreshold   = DMA_FIFOThreshold_HalfFull,
                    .DMA_MemoryBurst     = DMA_MemoryBurst_Single,
                    .DMA_PeripheralBurst = DMA_PeripheralBurst_Single,
                };

                // config data source and destination

                switch ((WaveAlign_e)(pWaveGen->u16WaveAlign))
                {
                    default: pWaveGen->u16WaveAlign = (u16)WaveAlign_12Bit_Right;
                    case WaveAlign_12Bit_Right:  // DAC_Align_12b_R
                    case WaveAlign_12Bit_Left:   // DAC_Align_12b_L
                    {
                        if (pWaveGen->u16WaveSize > ARRAY_SIZE(pWaveGen->u16WaveData))
                        {
                            pWaveGen->u16WaveSize = ARRAY_SIZE(pWaveGen->u16WaveData);
                        }

                        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
                        DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;

                        break;
                    }
                    case WaveAlign_8Bit_Right: {  // DAC_Align_8b_R

                        // 16bit -> 2 * 8bit

                        if (pWaveGen->u16WaveSize > ARRAY_SIZE(pWaveGen->u16WaveData) * 2)
                        {
                            pWaveGen->u16WaveSize = ARRAY_SIZE(pWaveGen->u16WaveData) * 2;
                        }

                        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
                        DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;

                        break;
                    }
                }

                DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&DAC->DHR12R1 + pWaveGen->u16WaveAlign + (DAC_Channel_x >> 4) * 3);
                DMA_InitStructure.DMA_Memory0BaseAddr    = (u32)(pWaveGen->u16WaveData);
                DMA_InitStructure.DMA_BufferSize         = pWaveGen->u16WaveSize;

                DMA_DeInit(DMA_Stream_x);
                DMA_Init(DMA_Stream_x, &DMA_InitStructure);
                DMA_Cmd(DMA_Stream_x, ENABLE);

                // TIM
                u16WaveSize = DMA_InitStructure.DMA_BufferSize;

                break;
            }
        }
    }

    //-----------------------------------------------------
    // TIM

    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

        u32 u32FreqUnit;
        switch ((WaveFreqUnit_e)(pWaveGen->u16WaveFreqUnit))
        {
            default:
            case WaveFreqUnit_Hz: u32FreqUnit = 1; break;
            case WaveFreqUnit_mHz: u32FreqUnit = 1e3; break;
        }

        u32 u32ActualFreq;
        u32 u32TargetFreq = pWaveGen->u32WaveTargetFreq * u16WaveSize / u32FreqUnit;

        if (PulseCalc(TIM_FRQ, u32TargetFreq, TIM_FRQ * 0.02,
                      (u16*)&TIM_TimeBaseStructure.TIM_Period,
                      (u16*)&TIM_TimeBaseStructure.TIM_Prescaler,
                      (u32*)&u32ActualFreq) == false)
        {
            return false;
        }

        pWaveGen->u32WaveActualFreq = u32ActualFreq * u32FreqUnit / u16WaveSize;

        TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(DACx_TIM, &TIM_TimeBaseStructure);

        TIM_SelectOutputTrigger(DACx_TIM, TIM_TRGOSource_Update);
        TIM_Cmd(DACx_TIM, ENABLE);

        // 当 DAC 的某通道由于 TIM 或其他未知原因而无法再配置, 可尝试在配置完成后调用软件触发
        // DAC_SoftwareTriggerCmd(DAC_Channel_x, ENABLE);
    }

    //-----------------------------------------------------
    //

    DAC_Cmd(DAC_Channel_x, ENABLE);

    return true;
}

//-----------------------------------------------------------------------------
// demo

static void WaveGenDemo(void)
{
    P(WaveGen1).u16WaveConfig = (u16)WaveConfig_Doit;
    P(WaveGen2).u16WaveConfig = (u16)WaveConfig_Doit;

    {
        switch (0)
        {
            default:
            case 0:
                P(WaveGen1).u32WaveTargetFreq = 10000;  // 10k
                P(WaveGen1).u16WaveType       = WaveType_Sine;
                break;
            case 1:
                P(WaveGen1).u32WaveTargetFreq = 20000;  // 20k
                P(WaveGen1).u16WaveType       = WaveType_Triangle;
                P(WaveGen1).u16WaveAmplitude  = WaveAmplitude_255;
                break;
        }
    }

    {
        P(WaveGen2).u32WaveTargetFreq = 1000;  // 1k
        P(WaveGen2).u16WaveType       = WaveType_Custom;

        // P(WaveGen2).u16WaveAlign = WaveAlign_12Bit_Left;
        P(WaveGen2).u16WaveAlign = WaveAlign_12Bit_Right;
        // P(WaveGen2).u16WaveAlign = WaveAlign_8Bit_Right;
        switch ((WaveAlign_e)P(WaveGen2).u16WaveAlign)
        {
            case WaveAlign_12Bit_Right:
            case WaveAlign_12Bit_Left:
                P(WaveGen2).u16WaveSize = 64;
                break;
            case WaveAlign_8Bit_Right:
                P(WaveGen2).u16WaveSize = 128;
                break;
        }

        s16 left  = P(WaveGen2).u16WaveSize * 0.3;
        s16 right = P(WaveGen2).u16WaveSize - left;

        for (int i = 0; i < P(WaveGen2).u16WaveSize; ++i)
        {
            u16 data;

            if (i < left)
            {
                data = (i - left) * (i - left);  // left
            }
            else if (i < right)
            {
                data = 1024;  // middle
            }
            else
            {
                data = (i - right) * (i - right);  // right
            }

            switch ((WaveAlign_e)P(WaveGen2).u16WaveAlign)
            {
                case WaveAlign_12Bit_Right:  // 0~4095
                    P(WaveGen2).u16WaveData[i] = data;
                    break;
                case WaveAlign_12Bit_Left:  // 16~65520
                    P(WaveGen2).u16WaveData[i] = data << 4;
                    break;
                case WaveAlign_8Bit_Right:  // 0~255
                    data /= 10;
                    (i & 0x1) ? (P(WaveGen2).u16WaveData[i / 2] |= data << 8) : (P(WaveGen2).u16WaveData[i / 2] = data);
                    break;
                default:
                    break;
            }
        }
    }
}
