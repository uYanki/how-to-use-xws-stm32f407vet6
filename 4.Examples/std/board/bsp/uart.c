#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "system/autoinit.h"

#include "pinmap.h"
#include "uart.h"

//-----------------------------------------------------------------------------
// configurations

#if 0  // template

#define CONFIG_UART_MODE            UART_MODE_RS485

#define CONFIG_UART_REDIRECT_PRINTF 1
#define CONFIG_UART_REDIRECT_SCANF  1

#endif

#define UART_MODE_RS232 1
#define UART_MODE_RS485 2

#ifndef CONFIG_UART_MODE
#define CONFIG_UART_MODE UART_MODE_RS232
#endif

//-----------------------------------------------------------------------------
// ports

#if CONFIG_UART_MODE == UART_MODE_RS232

// usart

#define UART_PORT              RS232_UART_PORT
#define UART_CLK               RS232_UART_CLK
#define UART_CLKEN             RS232_UART_CLKEN
#define UART_IRQn              USART1_IRQn
#define UART_IRQHandler        USART1_IRQHandler

// gpio

#define UART_TX_GPIO_CLK       RS232_TX_GPIO_CLK
#define UART_TX_GPIO_PORT      RS232_TX_GPIO_PORT
#define UART_TX_GPIO_PIN       RS232_TX_GPIO_PIN
#define UART_TX_GPIO_PINSRC    RS232_TX_GPIO_PINSRC
#define UART_TX_GPIO_AF        RS232_GPIO_AF

#define UART_RX_GPIO_CLK       RS232_RX_GPIO_CLK
#define UART_RX_GPIO_PORT      RS232_RX_GPIO_PORT
#define UART_RX_GPIO_PIN       RS232_RX_GPIO_PIN
#define UART_RX_GPIO_PINSRC    RS232_RX_GPIO_PINSRC
#define UART_RX_GPIO_AF        RS232_GPIO_AF

// dma

// 不同外设不同的 Stream 和 Channel, 详看数据手册

#define UART_TX_DMA_CLKEN      RCC_AHB1PeriphClockCmd
#define UART_TX_DMA_CLK        RCC_AHB1Periph_DMA2
#define UART_TX_DMA_CHANNEL    DMA_Channel_4
#define UART_TX_DMA_STREAM     DMA2_Stream7
#define UART_TX_DMA_FLAG       DMA_FLAG_TCIF7
#define UART_TX_DMA_IT         DMA_IT_TCIF7
#define UART_TX_DMA_IRQn       DMA2_Stream7_IRQn
#define UART_TX_DMA_IRQHandler DMA2_Stream7_IRQHandler

#define UART_RX_DMA_CLKEN      RCC_AHB1PeriphClockCmd
#define UART_RX_DMA_CLK        RCC_AHB1Periph_DMA2
#define UART_RX_DMA_CHANNEL    DMA_Channel_4
#define UART_RX_DMA_STREAM     DMA2_Stream5  // or DMA2_Stream2
#define UART_RX_DMA_FLAG       DMA_FLAG_TCIF5

#elif CONFIG_UART_MODE == UART_MODE_RS485

// uart

#define UART_PORT              RS485_UART_PORT
#define UART_CLK               RS485_UART_CLK
#define UART_CLKEN             RS485_UART_CLKEN
#define UART_IRQn              USART2_IRQn
#define UART_IRQHandler        USART2_IRQHandler

// gpio

#define UART_TX_GPIO_CLK       RS485_TX_GPIO_CLK
#define UART_TX_GPIO_PORT      RS485_TX_GPIO_PORT
#define UART_TX_GPIO_PIN       RS485_TX_GPIO_PIN
#define UART_TX_GPIO_PINSRC    RS485_TX_GPIO_PINSRC
#define UART_TX_GPIO_AF        RS485_GPIO_AF

#define UART_RX_GPIO_CLK       RS485_RX_GPIO_CLK
#define UART_RX_GPIO_PORT      RS485_RX_GPIO_PORT
#define UART_RX_GPIO_PIN       RS485_RX_GPIO_PIN
#define UART_RX_GPIO_PINSRC    RS485_RX_GPIO_PINSRC
#define UART_RX_GPIO_AF        RS485_GPIO_AF

#define UART_RTS_GPIO_CLK      RS485_RTS_GPIO_CLK
#define UART_RTS_GPIO_PORT     RS485_RTS_GPIO_PORT
#define UART_RTS_GPIO_PIN      RS485_RTS_GPIO_PIN

#define UART_SetTxDir()        RS485_SetTxDir()
#define UART_SetRxDir()        RS485_SetRxDir()

// dma

#define UART_TX_DMA_CLKEN      RCC_AHB1PeriphClockCmd
#define UART_TX_DMA_CLK        RCC_AHB1Periph_DMA1
#define UART_TX_DMA_CHANNEL    DMA_Channel_4
#define UART_TX_DMA_STREAM     DMA1_Stream6
#define UART_TX_DMA_FLAG       DMA_FLAG_TCIF6
#define UART_TX_DMA_IT         DMA_IT_TCIF6
#define UART_TX_DMA_IRQn       DMA1_Stream6_IRQn
#define UART_TX_DMA_IRQHandler DMA1_Stream6_IRQHandler

#define UART_RX_DMA_CLKEN      RCC_AHB1PeriphClockCmd
#define UART_RX_DMA_CLK        RCC_AHB1Periph_DMA1
#define UART_RX_DMA_CHANNEL    DMA_Channel_4
#define UART_RX_DMA_STREAM     DMA1_Stream5
#define UART_RX_DMA_FLAG       DMA_FLAG_TCIF5

#endif

// transmit what received
// #define CONFIG_UART_RXCBK(buffer, length) UART_Transmit_DMA(UART_RXBUF_ADDR, length, false)

//-----------------------------------------------------------------------------
// buffer

static u8 m_rxbuf[64] = {0};
static u8 m_txbuf[64] = {0};

#define UART_RXBUF_ADDR m_rxbuf
#define UART_RXBUF_SIZE sizeof(m_rxbuf)

#define UART_TXBUF_ADDR m_txbuf
#define UART_TXBUF_SIZE sizeof(m_txbuf)

//-----------------------------------------------------------------------------
// initialization

void UART_Config(USART_InitTypeDef* config)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(UART_TX_GPIO_CLK | UART_RX_GPIO_CLK, ENABLE);

        GPIO_PinAFConfig(UART_TX_GPIO_PORT, UART_TX_GPIO_PINSRC, UART_TX_GPIO_AF);
        GPIO_PinAFConfig(UART_RX_GPIO_PORT, UART_RX_GPIO_PINSRC, UART_RX_GPIO_AF);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;

        GPIO_InitStructure.GPIO_Pin = UART_TX_GPIO_PIN;
        GPIO_Init(UART_TX_GPIO_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = UART_RX_GPIO_PIN;
        GPIO_Init(UART_RX_GPIO_PORT, &GPIO_InitStructure);

#if CONFIG_UART_MODE == UART_MODE_RS485

        RCC_AHB1PeriphClockCmd(RS485_RTS_GPIO_CLK, ENABLE);

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

        GPIO_InitStructure.GPIO_Pin = UART_RTS_GPIO_PIN;
        GPIO_Init(UART_RTS_GPIO_PORT, &GPIO_InitStructure);

        UART_SetRxDir();

#endif
    }

    // uart
    {
        // USART_InitTypeDef USART_InitStructure;

        UART_CLKEN(UART_CLK, ENABLE);

        // USART_InitStructure.USART_BaudRate            = baudrate;
        // USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        // USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
        // USART_InitStructure.USART_Parity              = USART_Parity_No;
        // USART_InitStructure.USART_StopBits            = USART_StopBits_1;
        // USART_InitStructure.USART_WordLength          = USART_WordLength_8b;

        // USART_Init(UART_PORT, &USART_InitStructure);

        USART_Init(UART_PORT, config);
    }

    // enable uart
    USART_Cmd(UART_PORT, ENABLE);
}

void UART_DMA_Config(void)
{
    // dma
    {
        DMA_InitTypeDef DMA_InitStructure;

        UART_TX_DMA_CLKEN(UART_TX_DMA_CLK, ENABLE);
        UART_RX_DMA_CLKEN(UART_RX_DMA_CLK, ENABLE);

        DMA_DeInit(UART_TX_DMA_STREAM);
        while (DMA_GetCmdStatus(UART_TX_DMA_STREAM) != DISABLE)
            ;

        DMA_DeInit(UART_RX_DMA_STREAM);
        while (DMA_GetCmdStatus(UART_RX_DMA_STREAM) != DISABLE)
            ;

        // common

        DMA_InitStructure.DMA_MemoryBurst    = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  // byte
        DMA_InitStructure.DMA_MemoryInc      = DMA_MemoryInc_Enable;     // inc

        DMA_InitStructure.DMA_PeripheralBaseAddr = (u32) & (UART_PORT->DR);
        DMA_InitStructure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;

        DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;

        DMA_InitStructure.DMA_FIFOMode      = DMA_FIFOMode_Disable;  // unused
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;

        // tx dma
        {
            DMA_InitStructure.DMA_Mode            = DMA_Mode_Normal;  // oneshot
            DMA_InitStructure.DMA_Channel         = UART_TX_DMA_CHANNEL;
            DMA_InitStructure.DMA_DIR             = DMA_DIR_MemoryToPeripheral;
            DMA_InitStructure.DMA_Memory0BaseAddr = (u32)UART_TXBUF_ADDR;  // dat dest
            DMA_InitStructure.DMA_BufferSize      = 0;

            DMA_Init(UART_TX_DMA_STREAM, &DMA_InitStructure);
            DMA_Cmd(UART_TX_DMA_STREAM, DISABLE);
        }

        // rx dma
        {
            DMA_InitStructure.DMA_Mode            = DMA_Mode_Circular;
            DMA_InitStructure.DMA_Channel         = UART_RX_DMA_CHANNEL;
            DMA_InitStructure.DMA_DIR             = DMA_DIR_PeripheralToMemory;
            DMA_InitStructure.DMA_Memory0BaseAddr = (u32)UART_RXBUF_ADDR;  // dat src
            DMA_InitStructure.DMA_BufferSize      = UART_RXBUF_SIZE;

            DMA_Init(UART_RX_DMA_STREAM, &DMA_InitStructure);
            DMA_Cmd(UART_RX_DMA_STREAM, DISABLE);
        }
    }

    // nvic
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

        // uart
        {
            NVIC_InitStructure.NVIC_IRQChannel                   = UART_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
            NVIC_Init(&NVIC_InitStructure);
        }

        // dma
        {
            NVIC_InitStructure.NVIC_IRQChannel                   = UART_TX_DMA_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
            NVIC_Init(&NVIC_InitStructure);
        }
    }

    // interrupt
    {
        {
            // - USART_FLAG_TC: transmission complete, data has been sent out
            // - USART_FLAG_TXE: transmission data register empty, data move from data
            //        register to shift register, but data has not yet been sent out

            // disable irq
            USART_ITConfig(UART_PORT, USART_IT_RXNE, DISABLE);  // 接收不为空
            USART_ITConfig(UART_PORT, USART_IT_TXE, DISABLE);   // 发送为空

            // @note: `USART_FLAG_TC` is set after calling `USART_Init` or `USART_Cmd`
            // if not clear it, `USART_ITConfig` will trigger interrupt
            USART_ClearFlag(UART_PORT, USART_FLAG_TC);
            USART_ITConfig(UART_PORT, USART_IT_TC, DISABLE);  // 传输完成

            // enable irq
            USART_ITConfig(UART_PORT, USART_IT_IDLE, ENABLE);  // idle, 空闲
        }
        {
            // enable irq
            DMA_ClearITPendingBit(UART_TX_DMA_STREAM, UART_TX_DMA_IT);
            DMA_ITConfig(UART_TX_DMA_STREAM, DMA_IT_TC, ENABLE);
        }
    }

    // enable DMA request
    USART_DMACmd(UART_PORT, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
    // enable uart RX DMA
    DMA_Cmd(UART_RX_DMA_STREAM, ENABLE);
}

//-----------------------------------------------------------------------------

bool UART_Transmit_DMA(void* datsrc, u16 len, bool block /*阻塞式等待*/)
{
    if (block)
    {
        while (DMA_GetCurrDataCounter(UART_TX_DMA_STREAM))
            ;
    }
    else
    {
        if (DMA_GetCurrDataCounter(UART_TX_DMA_STREAM))
        {
            return false;
        }
    }

#if (CONFIG_UART_MODE == UART_MODE_RS485)
    // set tx dir
    UART_SetTxDir();
    sleep_us(50);
#endif

    DMA_Cmd(UART_TX_DMA_STREAM, DISABLE);

    // check if it is configurable
    while (DMA_GetCmdStatus(UART_TX_DMA_STREAM) != DISABLE)
        ;

#if 0
    DMA_MemoryTargetConfig(UART_TX_DMA_STREAM, (u32)datsrc, DMA_Memory_0);
    DMA_SetCurrDataCounter(UART_TX_DMA_STREAM, len);
#else
    UART_TX_DMA_STREAM->M0AR = (u32)datsrc;
    UART_TX_DMA_STREAM->NDTR = len;
#endif

    // transmit data
    DMA_Cmd(UART_TX_DMA_STREAM, ENABLE);

    return true;
}

//-----------------------------------------------------------------------------
// irq handler

void UART_TX_DMA_IRQHandler(void)
{
    if (DMA_GetITStatus(UART_TX_DMA_STREAM, UART_TX_DMA_IT))
    {
        // clear flag
        DMA_ClearITPendingBit(UART_TX_DMA_STREAM, UART_TX_DMA_IT);
        DMA_ClearFlag(UART_TX_DMA_STREAM, UART_TX_DMA_FLAG);
        // disable tx dma
        DMA_Cmd(UART_TX_DMA_STREAM, DISABLE);
        DMA_SetCurrDataCounter(UART_TX_DMA_STREAM, 0);
        // enable uart interrupt
        USART_ITConfig(UART_PORT, USART_IT_TC, ENABLE);
    }
}

void UART_IRQHandler(void)
{
    //------------------------------------------------
    // idle interrupt
    if (USART_GetITStatus(UART_PORT, USART_IT_IDLE) == SET)
    {
        // clear interrupt flag:
        // It is cleared by a software sequence (an read to the USART_SR register
        // followed by a read to the USART_DR register).
        USART_ClearITPendingBit(UART_PORT, USART_IT_IDLE);
        USART_ReceiveData(UART_PORT);

        DMA_ClearFlag(UART_RX_DMA_STREAM, UART_RX_DMA_FLAG);
        DMA_Cmd(UART_RX_DMA_STREAM, DISABLE);

        uint16_t length = UART_RXBUF_SIZE - DMA_GetCurrDataCounter(UART_RX_DMA_STREAM);

        UART_RXBUF_ADDR[length] = '\0';

#ifdef CONFIG_UART_RXCBK

        if (length > 0)
        {
            CONFIG_UART_RXCBK(UART_RXBUF_ADDR, length);
        }

#endif

        // receive data again
        DMA_SetCurrDataCounter(UART_RX_DMA_STREAM, UART_RXBUF_SIZE);
        DMA_Cmd(UART_RX_DMA_STREAM, ENABLE);
    }

    //------------------------------------------------
    // transmit complete interrupt
    if (USART_GetITStatus(UART_PORT, USART_IT_TC) == SET)
    {
        // disable interrupt
        USART_ITConfig(UART_PORT, USART_IT_TC, DISABLE);
        // clear flag
        USART_ClearFlag(UART_PORT, USART_FLAG_TC);
        USART_ClearITPendingBit(UART_PORT, USART_IT_TC);

#ifdef CONFIG_UART_TXCBK
        CONFIG_UART_TXCBK();
#endif

#if CONFIG_UART_MODE == UART_MODE_RS485
        // set rx mode
        sleep_us(160);
        UART_SetRxDir();
#endif
    }
}

//-----------------------------------------------------------------------------
// stdio redirect

#if CONFIG_UART_REDIRECT_PRINTF

#if defined(__CC_ARM)

#include <stdarg.h>

int $Super$$__2printf(const char* fmt, ...);

// precall
int $Sub$$__2printf(const char* fmt, ...)
{
    int len;

#if (CONFIG_UART_MODE == UART_MODE_RS485)
    // set tx dir
    UART_SetTxDir();
    sleep_us(60);
#endif

    // $Super$$__2printf("<*>");

    va_list args;

    va_start(args, fmt);
    len = vprintf(fmt, args);  // print
    va_end(args);

#if (CONFIG_UART_MODE == UART_MODE_RS485)
    // set tx dir
    sleep_us(150);
    UART_SetRxDir();
#endif

    return len;
}

#endif

int fputc(int ch, FILE* f)
{
    if (ch == '\n')
    {
        USART_SendData(UART_PORT, '\r');

        while (USART_GetFlagStatus(UART_PORT, USART_FLAG_TXE) == RESET)
            ;
    }

    USART_SendData(UART_PORT, (uint8_t)ch);

    while (USART_GetFlagStatus(UART_PORT, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}

#endif

#if CONFIG_UART_REDIRECT_SCANF

int fgetc(FILE* f)
{
    while (USART_GetFlagStatus(UART_PORT, USART_FLAG_RXNE) == RESET)
        ;

    return (int)USART_ReceiveData(UART_PORT);
}

#endif
