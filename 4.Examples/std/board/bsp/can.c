#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_can.h"

#include "system/autoinit.h"

#include "pinmap.h"
#include "can.h"
#include "usdk.defs.h"

/**
 * STM32F4 的 bxCAN 的主要特点有：
 * - 支持 CAN 协议 2.0A 和 2.0B 主动模式
 * - 波特率最高达 1Mbps
 * - 支持时间触发通信
 * - 具有 3 个发送邮箱
 * - 具有 3 级深度的 2 个接收 FIFO
 * - 可变的过滤器组（28 个，CAN1 和 CAN2 共享）
 */

/**
 * CAN1: 主 bxCAN, 它负责管理在从bxCAN和512字节的SRAM存储器之间的通.
 * CAN2: 从 bxCAN, 它不能直接访问SRAM存储器.
 *
 * 2 个 bxCAN 模块共享 512 字节的 SRAM 存储器, 因此在使用 CAN2 (从)
 * 时需要先使能 CAN1(主) 的时钟, 否则 CAN2 只发不收.
 */

//------------------------------------------------------------
//

bool can_init(CAN_TypeDef* CANx, u32 bitrate)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

        if (CANx == CAN1)
        {
            RCC_AHB1PeriphClockCmd(CAN1_TX_GPIO_CLK | CAN1_RX_GPIO_CLK, ENABLE);

            GPIO_PinAFConfig(CAN1_TX_GPIO_PORT, CAN1_TX_GPIO_PINSRC, CAN1_GPIO_AF);
            GPIO_PinAFConfig(CAN1_RX_GPIO_PORT, CAN1_RX_GPIO_PINSRC, CAN1_GPIO_AF);

            GPIO_InitStructure.GPIO_Pin = CAN1_TX_GPIO_PIN;
            GPIO_Init(CAN1_TX_GPIO_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = CAN1_RX_GPIO_PIN;
            GPIO_Init(CAN1_RX_GPIO_PORT, &GPIO_InitStructure);

            // f103: GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
        }
        else if (CANx == CAN2)
        {
            RCC_AHB1PeriphClockCmd(CAN2_TX_GPIO_CLK | CAN2_RX_GPIO_CLK, ENABLE);

            GPIO_PinAFConfig(CAN2_TX_GPIO_PORT, CAN2_TX_GPIO_PINSRC, CAN2_GPIO_AF);
            GPIO_PinAFConfig(CAN2_RX_GPIO_PORT, CAN2_RX_GPIO_PINSRC, CAN2_GPIO_AF);

            GPIO_InitStructure.GPIO_Pin = CAN2_TX_GPIO_PIN;
            GPIO_Init(CAN2_TX_GPIO_PORT, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = CAN2_RX_GPIO_PIN;
            GPIO_Init(CAN2_RX_GPIO_PORT, &GPIO_InitStructure);
        }
        else
        {
            return false;
        }
    }
    // can
    {
        CAN_InitTypeDef CAN_InitStructure;

        if (CANx == CAN1)
        {
            RCC_APB1PeriphClockCmd(CAN1_CLK, ENABLE);
        }
        else  // if (CANx == CAN2)
        {
            RCC_APB1PeriphClockCmd(CAN1_CLK, ENABLE);
            RCC_APB1PeriphClockCmd(CAN2_CLK, ENABLE);
        }

        CAN_InitStructure.CAN_TTCM = DISABLE;
        CAN_InitStructure.CAN_ABOM = DISABLE;
        CAN_InitStructure.CAN_AWUM = DISABLE;
        CAN_InitStructure.CAN_NART = DISABLE;
        CAN_InitStructure.CAN_RFLM = DISABLE;
        CAN_InitStructure.CAN_TXFP = DISABLE;

        /** CAN Baudrate (CAN clocked at APB1)
         *
         * - stm32f407: APB1 = 42MHz
         *
         *     bitrate = 42e6 / ( 6 * (1+BS1+BS2) )
         *
         */

        switch (bitrate)
        {
            case 1000000:  // 1M bps

                CAN_InitStructure.CAN_SJW       = CAN_SJW_1tq;
                CAN_InitStructure.CAN_BS1       = CAN_BS1_3tq;
                CAN_InitStructure.CAN_BS2       = CAN_BS2_3tq;
                CAN_InitStructure.CAN_Prescaler = 6;

                break;

            default:
            case 500000:  // 500k bps

                CAN_InitStructure.CAN_SJW       = CAN_SJW_1tq;
                CAN_InitStructure.CAN_BS1       = CAN_BS1_6tq;
                CAN_InitStructure.CAN_BS2       = CAN_BS2_7tq;
                CAN_InitStructure.CAN_Prescaler = 6;

                break;

            case 250000:  // 250k bps

                CAN_InitStructure.CAN_SJW       = CAN_SJW_1tq;
                CAN_InitStructure.CAN_BS1       = CAN_BS1_6tq;
                CAN_InitStructure.CAN_BS2       = CAN_BS2_7tq;
                CAN_InitStructure.CAN_Prescaler = 12;

                break;

            case 125000:  // 125k bps

                CAN_InitStructure.CAN_SJW       = CAN_SJW_1tq;
                CAN_InitStructure.CAN_BS1       = CAN_BS1_6tq;
                CAN_InitStructure.CAN_BS2       = CAN_BS2_7tq;
                CAN_InitStructure.CAN_Prescaler = 24;

                break;
        }

        CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

        CAN_Init(CANx, &CAN_InitStructure);
    }
    // filter
    {
        /**
         * Bits 13:8 CAN2SB[5:0]: CAN2 start bank (default=14)
         *  These bits are set and cleared by software. They define the start bank for the CAN2
         *  interface (Slave) in the range 0 to 27.
         *  Note: When CAN2SB[5:0] = 28d, all the filters to CAN1 can be used.
         *  When CAN2SB[5:0] is set to 0, no filters are assigned to CAN1.
         *
         * https://shequ.stmicroelectronics.cn/thread-614053-1-1.html
         *  当值 n 为0时，CAN1、CAN2可用全部28滤波器。
         *  当  1 <= n <= 27 时，CAN1 可使用编号为 0 ~ n-1 滤波器，CAN2 可使用编号为 n-1 ~ 27的滤波器，符合文档描述。
         *  当 28 <= n <= 58 时，CAN1、CAN2 可用全部28滤波器。
         *  当 59 <= n <= 63 时，CAN1 可使用编号为 0 ~ n-59 滤波器，CAN2 可使用编号为 n-59 ~ 27 的滤波器。
         */

        CAN_FilterInitTypeDef CAN_FilterInitStructure;

        CAN_FilterInitStructure.CAN_FilterMode  = CAN_FilterMode_IdMask;
        CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;

#if 0  // write CAN2SB
        CLRBITS(CAN1->FMR, 8, 6);
        CAN1->FMR |= (28 << 8);
#endif

        // if Frame-ID & FilterMaskId = FilterId, put it into RX FIFO

        if (CANx == CAN1)
        {
            CAN_FilterInitStructure.CAN_FilterNumber         = 0;
            CAN_FilterInitStructure.CAN_FilterIdHigh         = 0x0000;
            CAN_FilterInitStructure.CAN_FilterIdLow          = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0x0000;
            CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
            CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;  // 关联 FIFO0
            // CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 1;  // 关联 FIFO1
            CAN_FilterInit(&CAN_FilterInitStructure);

            // 多组过滤器
            // CAN_FilterInitStructure.CAN_FilterNumber         = 1;
            // CAN_FilterInitStructure.CAN_FilterIdHigh         = 0x0000;
            // CAN_FilterInitStructure.CAN_FilterIdLow          = 0x0000;
            // CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = 0x0000;
            // CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0x0000;
            // CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
            // CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
            // CAN_FilterInit(&CAN_FilterInitStructure);
            // ...
        }
        else  // if (CANx == CAN2)
        {
            CAN_FilterInitStructure.CAN_FilterNumber         = GETBITS(CAN1->FMR, 8, 6);  // read CAN2SB
            CAN_FilterInitStructure.CAN_FilterIdHigh         = 0x0000;
            CAN_FilterInitStructure.CAN_FilterIdLow          = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = 0x0000;
            CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0x0000;
            CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
            CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;  // 关联 FIFO0
            // CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 1;  // 关联 FIFO1
            CAN_FilterInit(&CAN_FilterInitStructure);
        }
    }
    // nvic
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        if (CANx == CAN1)
        {
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
            NVIC_InitStructure.NVIC_IRQChannel                   = CAN1_RX0_IRQn;
            NVIC_Init(&NVIC_InitStructure);

            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
            NVIC_InitStructure.NVIC_IRQChannel                   = CAN1_RX1_IRQn;
            NVIC_Init(&NVIC_InitStructure);
        }
        else  // if (CANx == CAN2)
        {
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
            NVIC_InitStructure.NVIC_IRQChannel                   = CAN2_RX0_IRQn;
            NVIC_Init(&NVIC_InitStructure);

            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
            NVIC_InitStructure.NVIC_IRQChannel                   = CAN2_RX1_IRQn;
            NVIC_Init(&NVIC_InitStructure);
        }

        // Enable FIFO message pending Interrupt
        CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
        CAN_ITConfig(CANx, CAN_IT_FMP1, ENABLE);

        // void CAN1_RX0_IRQHandler(void)
        // void CAN1_RX1_IRQHandler(void)
        // void CAN2_RX0_IRQHandler(void)
        // void CAN2_RX1_IRQHandler(void)
    }

    return true;
}

//------------------------------------------------------------------------------
//

/* msg: CanTxMsg or CanRxMsg */
void can_display_msg(const char* tag, void* msg)
{
    CanTxMsg* ctx = (CanTxMsg*)msg;

    uint8_t idx = 0;

    u32 id = ctx->StdId;  //  frame id

    if (ctx->IDE == CAN_ID_EXT)
    {
        id |= ctx->ExtId << 11;
    }

    printf("%s", tag);

    printf(" - 0x%X (%d,%d) :", id, ctx->RTR, ctx->DLC);

    while (idx < ctx->DLC)
    {
        printf("%02X ", ctx->Data[idx++]);
    }

    printf("\n");
}

//------------------------------------------------------------------------------
//

#if 0  // display rx frame 

void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    can_display_msg("<+> CAN1 RX0", &RxMessage);
}

void CAN1_RX1_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN1, CAN_FIFO1, &RxMessage);
    can_display_msg("<+> CAN1 RX1", &RxMessage);
}

void CAN2_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
    can_display_msg("<-> CAN2 RX0", &RxMessage);
}

void CAN2_RX1_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN2, CAN_FIFO1, &RxMessage);
    can_display_msg("<-> CAN2 RX1", &RxMessage);
}

#endif
