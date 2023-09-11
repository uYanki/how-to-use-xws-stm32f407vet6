#include "main.h"

#include <stdio.h>

///

// gpio & usart

#if 0

#define USART_PORT           RS232_UART_PORT
#define USART_CLK            RS232_UART_CLK
#define USART_CLKEN          RS232_UART_CLKEN
#define USART_IRQn           USART1_IRQn
#define USART_IRQHandler     USART1_IRQHandler

#define USART_TX_GPIO_CLK    RS232_TX_GPIO_CLK
#define USART_TX_GPIO_PORT   RS232_TX_GPIO_PORT
#define USART_TX_GPIO_PIN    RS232_TX_GPIO_PIN
#define USART_TX_GPIO_PINSRC RS232_TX_GPIO_PINSRC
#define USART_TX_GPIO_AF     RS232_GPIO_AF

#define USART_RX_GPIO_CLK    RS232_RX_GPIO_CLK
#define USART_RX_GPIO_PORT   RS232_RX_GPIO_PORT
#define USART_RX_GPIO_PIN    RS232_RX_GPIO_PIN
#define USART_RX_GPIO_PINSRC RS232_RX_GPIO_PINSRC
#define USART_RX_GPIO_AF     RS232_GPIO_AF

#else

#define USART_PORT           RS485_UART_PORT
#define USART_CLK            RS485_UART_CLK
#define USART_CLKEN          RS485_UART_CLKEN
#define USART_IRQn           USART2_IRQn
#define USART_IRQHandler     USART2_IRQHandler

#define USART_TX_GPIO_CLK    RS485_TX_GPIO_CLK
#define USART_TX_GPIO_PORT   RS485_TX_GPIO_PORT
#define USART_TX_GPIO_PIN    RS485_TX_GPIO_PIN
#define USART_TX_GPIO_PINSRC RS485_TX_GPIO_PINSRC
#define USART_TX_GPIO_AF     RS485_GPIO_AF

#define USART_RX_GPIO_CLK    RS485_RX_GPIO_CLK
#define USART_RX_GPIO_PORT   RS485_RX_GPIO_PORT
#define USART_RX_GPIO_PIN    RS485_RX_GPIO_PIN
#define USART_RX_GPIO_PINSRC RS485_RX_GPIO_PINSRC
#define USART_RX_GPIO_AF     RS485_GPIO_AF

#define USART_RTS_GPIO_CLK   RS485_RTS_GPIO_CLK
#define USART_RTS_GPIO_PORT  RS485_RTS_GPIO_PORT
#define USART_RTS_GPIO_PIN   RS485_RTS_GPIO_PIN
#define USART_SetTxDir()     RS485_SetTxDir()
#define USART_SetRxDir()     RS485_SetRxDir()

#endif

// dma

#define USART_DR_BASE        &(USART_PORT->DR)

#define USART_TX_DMA_CLKEN   RCC_AHB1PeriphClockCmd
#define USART_TX_DMA_CLK     RCC_AHB1Periph_DMA2
#define USART_TX_DMA_CHANNEL DMA_Channel_4
#define USART_TX_DMA_STREAM  DMA2_Stream7
#define USART_TX_DMA_TCIF    DMA_IT_TCIF7  // transmission complete interrupt flag

#define USART_RX_DMA_CLKEN   RCC_AHB1PeriphClockCmd
#define USART_RX_DMA_CLK     RCC_AHB1Periph_DMA2
#define USART_RX_DMA_CHANNEL DMA_Channel_4
#define USART_RX_DMA_STREAM  DMA2_Stream5

////

static u8 m_rxbuf[64] = {0};
static u8 m_txbuf[64] = {0};

#define USART_RXBUF_ADDR m_rxbuf
#define USART_RXBUF_SIZE sizeof(m_rxbuf)

#define USART_TXBUF_ADDR m_rxbuf

///

void UART_Config(u32 baudrate)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(USART_TX_GPIO_CLK | USART_RX_GPIO_CLK, ENABLE);

        GPIO_PinAFConfig(USART_TX_GPIO_PORT, USART_TX_GPIO_PINSRC, USART_TX_GPIO_AF);
        GPIO_PinAFConfig(USART_RX_GPIO_PORT, USART_RX_GPIO_PINSRC, USART_RX_GPIO_AF);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;

        GPIO_InitStructure.GPIO_Pin = USART_TX_GPIO_PIN;
        GPIO_Init(USART_TX_GPIO_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = USART_RX_GPIO_PIN;
        GPIO_Init(USART_RX_GPIO_PORT, &GPIO_InitStructure);

#ifdef USART_RTS_GPIO_CLK

        RCC_AHB1PeriphClockCmd(RS485_RTS_GPIO_CLK, ENABLE);

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

        GPIO_InitStructure.GPIO_Pin = USART_RTS_GPIO_PIN;
        GPIO_Init(USART_RTS_GPIO_PORT, &GPIO_InitStructure);

        USART_SetRxDir();

#endif
    }

    // uart
    {
        USART_InitTypeDef USART_InitStructure;

        USART_CLKEN(USART_CLK, ENABLE);

        USART_InitStructure.USART_BaudRate            = baudrate;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
        USART_InitStructure.USART_Parity              = USART_Parity_No;
        USART_InitStructure.USART_StopBits            = USART_StopBits_1;
        USART_InitStructure.USART_WordLength          = USART_WordLength_8b;

        USART_Init(USART_PORT, &USART_InitStructure);
    }

    // nvic
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannel                   = USART_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;

        NVIC_Init(&NVIC_InitStructure);
    }

    // dma
    {
        DMA_InitTypeDef DMA_InitStructure;

        USART_TX_DMA_CLKEN(USART_TX_DMA_CLK, ENABLE);
        USART_RX_DMA_CLKEN(USART_RX_DMA_CLK, ENABLE);

        // common

        DMA_InitStructure.DMA_MemoryBurst    = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  // byte
        DMA_InitStructure.DMA_MemoryInc      = DMA_MemoryInc_Enable;     // inc

        DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)USART_DR_BASE;
        DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;

        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;

        DMA_InitStructure.DMA_FIFOMode      = DMA_FIFOMode_Disable;  // unused
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;

        // tx dma

        DMA_Cmd(USART_TX_DMA_STREAM, DISABLE);

        DMA_InitStructure.DMA_Mode            = DMA_Mode_Normal;  // oneshot
        DMA_InitStructure.DMA_Channel         = USART_TX_DMA_CHANNEL;
        DMA_InitStructure.DMA_DIR             = DMA_DIR_MemoryToPeripheral;
        DMA_InitStructure.DMA_Memory0BaseAddr = (u32)USART_TXBUF_ADDR;  // dat dest
        DMA_InitStructure.DMA_BufferSize      = 0;

        DMA_Init(USART_TX_DMA_STREAM, &DMA_InitStructure);

        // rx dma

        DMA_Cmd(USART_RX_DMA_STREAM, DISABLE);

        DMA_InitStructure.DMA_Mode            = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Channel         = USART_RX_DMA_CHANNEL;
        DMA_InitStructure.DMA_DIR             = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_Memory0BaseAddr = (u32)USART_RXBUF_ADDR;  // dat src
        DMA_InitStructure.DMA_BufferSize      = USART_RXBUF_SIZE;

        DMA_Init(USART_RX_DMA_STREAM, &DMA_InitStructure);
    }

    // enable idle irq
    USART_ITConfig(USART_PORT, USART_IT_TC, ENABLE);
    USART_ITConfig(USART_PORT, USART_IT_IDLE, ENABLE);
    // enable uart DMA request
    USART_DMACmd(USART_PORT, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
    // enable uart RX DMA
    DMA_Cmd(USART_RX_DMA_STREAM, ENABLE);
    // enable uart
    USART_Cmd(USART_PORT, ENABLE);
}

void USART_Transmit_DMA(void* datsrc, u16 len)
{
#ifdef USART_RTS_GPIO_CLK
    USART_SetTxDir();
#endif

    // transmit data
    DMA_Cmd(USART_TX_DMA_STREAM, DISABLE);

#if 0
    DMA_MemoryTargetConfig(USART_TX_DMA_STREAM, (u32)datsrc, DMA_Memory_0);
    DMA_SetCurrDataCounter(USART_TX_DMA_STREAM, len);
#else
    USART_TX_DMA_STREAM->M0AR = (u32)datsrc;
    USART_TX_DMA_STREAM->NDTR = len;
#endif

    DMA_Cmd(USART_TX_DMA_STREAM, ENABLE);

#if 0  // clear flag in USART_IRQHandler

    while (USART_GetFlagStatus(USART_PORT, USART_FLAG_TC) == RESET)
        ;  // necessary, otherwise the last bit of data is incorrect

    // clear flag
    DMA_ClearFlag(USART_TX_DMA_STREAM, USART_TX_DMA_TCIF);
    USART_ClearFlag(USART_PORT, USART_FLAG_TC);

#ifdef USART_RTS_GPIO_CLK
    USART_SetRxDir();
#endif

#endif
}

void USART_IRQHandler(void)
{
    // idle irq
    if (USART_GetITStatus(USART_PORT, USART_IT_IDLE) == SET)
    {
        uint16_t remaining = DMA_GetCurrDataCounter(USART_RX_DMA_STREAM);

        // receive data
        DMA_Cmd(USART_RX_DMA_STREAM, DISABLE);
        DMA_SetCurrDataCounter(USART_RX_DMA_STREAM, USART_RXBUF_SIZE);
        DMA_Cmd(USART_RX_DMA_STREAM, ENABLE);

        // transmit data (debug only)
        USART_Transmit_DMA(USART_RXBUF_ADDR, USART_RXBUF_SIZE - remaining);
    }
    // transmit complete
    else if (USART_GetITStatus(USART_PORT, USART_IT_TC) == SET)
    {
        // clear flag
        DMA_ClearFlag(USART_TX_DMA_STREAM, USART_TX_DMA_TCIF);
        USART_ClearFlag(USART_PORT, USART_FLAG_TC);

#ifdef USART_RTS_GPIO_CLK
        USART_SetRxDir();
#endif
    }

    // clear interrupt flag:
    // It is cleared by a software sequence (an read to the USART_SR register
    // followed by a read to the USART_DR register).
    // @note: the compiler will not optimize variables modified by volatile
    if (USART_PORT->SR & 0x1F)
    {
        // for
        // - PE: Parity error
        // - FE: Framing error
        // - NF: Noise detected flag
        // - ORE: Overrun error
        // - IDLE: IDLE line detected
        USART_PORT->DR;
    }
}

///

#ifndef USART_RTS_GPIO_CLK

#define print printf

#else

#define print(...)           \
    do {                     \
        USART_SetTxDir();    \
        printf(__VA_ARGS__); \
        USART_SetRxDir();    \
    } while (0)

#endif

int fputc(int ch, FILE* f)
{
    USART_SendData(USART_PORT, (uint8_t)ch);

    while (USART_GetFlagStatus(USART_PORT, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}

int fgetc(FILE* f)
{
    while (USART_GetFlagStatus(USART_PORT, USART_FLAG_RXNE) == RESET)
        ;

    return (int)USART_ReceiveData(USART_PORT);
}

///

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    UART_Config(115200);

    Led_Init();
    Key_Init();

    while (1)
    {
        if (KEY_IS_PRESS(KEY1_GPIO_PORT, KEY1_GPIO_PIN))
        {
            USART_Transmit_DMA("hello\r\n", 7);
        }
    }
}

// mbrtu: https://blog.csdn.net/qq_20222919/article/details/109110998
