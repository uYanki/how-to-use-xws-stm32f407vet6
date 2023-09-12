#include "main.h"

#include <stdio.h>
#include <stdbool.h>

///

#define USART_MODE_NORMAL            1
#define USART_MODE_RS232             2
#define USART_MODE_RS422             3
#define USART_MODE_RS485             4

#define CONFIG_USART_DEBUG           1

#define CONFIG_USART_MODE            USART_MODE_RS232
// #define CONFIG_USART_MODE            USART_MODE_RS485

#define CONFIG_USART_REDIRECT_PRINTF 1
#define CONFIG_USART_REDIRECT_SCANF  1

#ifndef CONFIG_USART_MODE
#define CONFIG_USART_MODE USART_MODE_NORMAL
#endif

#if 0  // debug only
		__disable_irq();
		print("USART_IT_IDLE");
		print("\r\n");
		__enable_irq();
#endif

// gpio & usart

#if CONFIG_USART_MODE == USART_MODE_RS232

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

#elif CONFIG_USART_MODE == USART_MODE_RS485

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

#else

#error "unsupported port"

#endif

// dma

#define DMA_Flags(stream)    (DMA_FLAG_TCIF##stream | DMA_FLAG_FEIF##stream | DMA_FLAG_DMEIF##stream | DMA_FLAG_TEIF##stream | DMA_FLAG_HTIF##stream)

#define USART_DR_BASE        &(USART_PORT->DR)

#define USART_TX_DMA_CLKEN   RCC_AHB1PeriphClockCmd
#define USART_TX_DMA_CLK     RCC_AHB1Periph_DMA2
#define USART_TX_DMA_CHANNEL DMA_Channel_4
#define USART_TX_DMA_STREAM  DMA2_Stream7
#define USART_TX_DMA_FLAG    DMA_Flags(7)

#define USART_RX_DMA_CLKEN   RCC_AHB1PeriphClockCmd
#define USART_RX_DMA_CLK     RCC_AHB1Periph_DMA2
#define USART_RX_DMA_CHANNEL DMA_Channel_4
#define USART_RX_DMA_STREAM  DMA2_Stream5
#define USART_RX_DMA_FLAG    DMA_Flags(4)

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

#if CONFIG_USART_MODE == USART_MODE_RS485

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

        DMA_DeInit(USART_TX_DMA_STREAM);
        while (DMA_GetCmdStatus(USART_TX_DMA_STREAM) != DISABLE)
            ;

        DMA_DeInit(USART_RX_DMA_STREAM);
        while (DMA_GetCmdStatus(USART_RX_DMA_STREAM) != DISABLE)
            ;

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

        DMA_InitStructure.DMA_Mode            = DMA_Mode_Normal;  // oneshot
        DMA_InitStructure.DMA_Channel         = USART_TX_DMA_CHANNEL;
        DMA_InitStructure.DMA_DIR             = DMA_DIR_MemoryToPeripheral;
        DMA_InitStructure.DMA_Memory0BaseAddr = (u32)USART_TXBUF_ADDR;  // dat dest
        DMA_InitStructure.DMA_BufferSize      = 0;

        DMA_Init(USART_TX_DMA_STREAM, &DMA_InitStructure);
        DMA_Cmd(USART_TX_DMA_STREAM, DISABLE);

        // rx dma

        DMA_InitStructure.DMA_Mode            = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Channel         = USART_RX_DMA_CHANNEL;
        DMA_InitStructure.DMA_DIR             = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_Memory0BaseAddr = (u32)USART_RXBUF_ADDR;  // dat src
        DMA_InitStructure.DMA_BufferSize      = USART_RXBUF_SIZE;

        DMA_Init(USART_RX_DMA_STREAM, &DMA_InitStructure);
        DMA_Cmd(USART_RX_DMA_STREAM, DISABLE);
    }

    // enable uart
    USART_Cmd(USART_PORT, ENABLE);

    // interrupt ( recommend set it after `USART_Cmd` )
    {
        // disable irq
        USART_ITConfig(USART_PORT, USART_IT_RXNE, DISABLE);
        USART_ITConfig(USART_PORT, USART_IT_TXE, DISABLE);

        // enable irq
        // @note: `USART_FLAG_TC` is set after calling `USART_Init` or `USART_Cmd`
        // if not clear it, `USART_ITConfig` will trigger interrupt
        USART_ClearFlag(USART_PORT, USART_FLAG_TC);
        USART_ITConfig(USART_PORT, USART_IT_TC, ENABLE);
        USART_ITConfig(USART_PORT, USART_IT_IDLE, ENABLE);  // idle
    }

    // enable DMA request
    USART_DMACmd(USART_PORT, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
    // enable uart RX DMA
    DMA_Cmd(USART_RX_DMA_STREAM, ENABLE);
}

bool USART_Transmit_DMA(void* datsrc, u16 len, bool block /*阻塞式等待*/)
{
    // - USART_FLAG_TC: transmission complete, data has been sent out
    // - USART_FLAG_TXE: transmission data register empty, data move from data
    //        register to shift register, but data has not yet been sent out

    if (block)
    {
        while (USART_GetFlagStatus(USART_PORT, USART_FLAG_TXE) == RESET)
            ;
    }
    else
    {
        // if (DMA_GetFlagStatus(USART_TX_DMA_STREAM, DMA_FLAG_TEIFx) == SET) // 丢数据
        if (USART_GetFlagStatus(USART_PORT, USART_FLAG_TXE) == RESET)
        {
            return false;
        }
    }

#if CONFIG_USART_MODE == USART_MODE_RS485
    // set tx mode
    USART_SetTxDir();
#endif

    DMA_Cmd(USART_TX_DMA_STREAM, DISABLE);

    // check if it is configurable
    while (DMA_GetCmdStatus(USART_TX_DMA_STREAM) != DISABLE)
        ;

#if 0
    DMA_MemoryTargetConfig(USART_TX_DMA_STREAM, (u32)datsrc, DMA_Memory_0);
    DMA_SetCurrDataCounter(USART_TX_DMA_STREAM, len);
#else
    USART_TX_DMA_STREAM->M0AR = (u32)datsrc;
    USART_TX_DMA_STREAM->NDTR = len;
#endif

    // transmit data
    DMA_Cmd(USART_TX_DMA_STREAM, ENABLE);

    return true;
}

void USART_IRQHandler(void)
{
    // idle interrupt
    if (USART_GetITStatus(USART_PORT, USART_IT_IDLE) == SET)
    {
        // clear interrupt flag:
        // It is cleared by a software sequence (an read to the USART_SR register
        // followed by a read to the USART_DR register).
        USART_ClearITPendingBit(USART_PORT, USART_IT_IDLE);
        USART_ReceiveData(USART_PORT);

        uint16_t length = USART_RXBUF_SIZE - DMA_GetCurrDataCounter(USART_RX_DMA_STREAM);

        DMA_Cmd(USART_RX_DMA_STREAM, DISABLE);
        DMA_ClearFlag(USART_RX_DMA_STREAM, USART_RX_DMA_FLAG);

#if CONFIG_USART_DEBUG  // debug only
			
        if (length > 0)
        {
            // transmit what receive
            USART_Transmit_DMA(USART_RXBUF_ADDR, length, true);
        }
				
#endif

        // receive data
        DMA_SetCurrDataCounter(USART_RX_DMA_STREAM, USART_RXBUF_SIZE);
        DMA_Cmd(USART_RX_DMA_STREAM, ENABLE);
    }
    // transmit complete interrupt
    if (USART_GetITStatus(USART_PORT, USART_IT_TC) == SET)
    {
        // clear flag
        USART_ClearITPendingBit(USART_PORT, USART_IT_TC);
        USART_ClearFlag(USART_PORT, USART_FLAG_TC);
        DMA_ClearFlag(USART_TX_DMA_STREAM, USART_TX_DMA_FLAG);
#if CONFIG_USART_MODE == USART_MODE_RS485
        // set rx mode
        USART_SetRxDir();
#endif
    }
}

///

#if CONFIG_USART_MODE == USART_MODE_RS485

#if 0

#define print(...)           \
    do {                     \
        USART_SetTxDir();    \
        printf(__VA_ARGS__); \
        USART_SetRxDir();    \
    } while (0)

#else

extern void $Super$$printf(void);

void $Sub$$printf(void)
{
    USART_SetTxDir();
    $Super$$printf();  // 调用原函数
    // USART_SetRxDir();
}

#endif

#endif

///

#if CONFIG_USART_REDIRECT_PRINTF

#define println(fmt, arg...) printf(fmt "\r\n", ##arg)

int fputc(int ch, FILE* f)
{
    USART_SendData(USART_PORT, (uint8_t)ch);

    while (USART_GetFlagStatus(USART_PORT, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}

#endif

#if CONFIG_USART_REDIRECT_SCANF

int fgetc(FILE* f)
{
    while (USART_GetFlagStatus(USART_PORT, USART_FLAG_RXNE) == RESET)
        ;

    return (int)USART_ReceiveData(USART_PORT);
}

#endif

///

// GUN - Extension: https://blog.csdn.net/shangzh/article/details/39398577

#if defined(__GNUC__)
// get minimum  value ( with type check )
#define _min(x, y) ({ const typeof(x) __x = (x); const typeof(y) __y = (y); (void) (&__x == &__y); __x < __y ? __x: __y; }) 
#else
#define _min(x, y) ((x) < (y) ? (x) : (y))
#endif

/// MDK - extension

// 补丁函数(闭包)

void sayhello(void)
{
    printf("hello");
}

void $Sub$$sayhello(void)
{
    // MDK 拓展语法: https://zhuanlan.zhihu.com/p/145219269
    // μVision Help -> search `USE of $Sub and $Super`
    // position: Compiler Getting Started Guide. Hardware initialization.

    // before
    printf("[");

    // 调用原函数
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern void $Super$$sayhello(void);
    $Super$$sayhello();
#elif defined(__ICCARM__) || defined(__GNUC__)
    extern void sayhello(void);
    sayhello();
#endif

    // after
    printf("]\r\n");
}

///

// clang-format off
// clang-format on

///

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    UART_Config(115200);

    Led_Init();
    Key_Init();

    {
			  // get minimum
        println("%d", _min(12, 13));

			  // array init
        u8 array[10] = {
            [0] = 10,
            [3] = 50,
#if defined(__GNUC__)
            [6 ... 8] = 100,
#endif
        };
        println("%d,%d,%d", array[0], array[3], array[7]);
    }

    u16 t = 0;

    while (1)
    {
        if (KEY_IS_PRESS(KEY1_GPIO_PORT, KEY1_GPIO_PIN))
        {
            if (++t == 0)
            {
#if 1
                sayhello();
#else
                USART_Transmit_DMA("hello\r\n", 7, false);
#endif
            }
        }
        else
        {
            t = 0;
        }
    }
}

// mbrtu: https://blog.csdn.net/qq_20222919/article/details/109110998
