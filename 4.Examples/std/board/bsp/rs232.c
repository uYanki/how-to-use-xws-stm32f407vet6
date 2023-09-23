#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "system/autoinit.h"

#include "pinmap.h"
#include "rs232.h"

//------------------------------------------------------------------------------
//

void rs232_init(USART_InitTypeDef* params)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHB1PeriphClockCmd(RS232_TX_GPIO_CLK | RS232_RX_GPIO_CLK, ENABLE);

        GPIO_PinAFConfig(RS232_TX_GPIO_PORT, RS232_TX_GPIO_PINSRC, RS232_GPIO_AF);
        GPIO_PinAFConfig(RS232_RX_GPIO_PORT, RS232_RX_GPIO_PINSRC, RS232_GPIO_AF);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;

        GPIO_InitStructure.GPIO_Pin = RS232_TX_GPIO_PIN;
        GPIO_Init(RS232_TX_GPIO_PORT, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = RS232_RX_GPIO_PIN;
        GPIO_Init(RS232_RX_GPIO_PORT, &GPIO_InitStructure);
    }

    // uart
    {
        // USART_InitTypeDef USART_InitStructure;

        RS232_UART_CLKEN(RS232_UART_CLK, ENABLE);

        // USART_InitStructure.USART_BaudRate            = baudrate;
        // USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        // USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
        // USART_InitStructure.USART_Parity              = USART_Parity_No;
        // USART_InitStructure.USART_StopBits            = USART_StopBits_1;
        // USART_InitStructure.USART_WordLength          = USART_WordLength_8b;

        USART_Init(RS232_UART_PORT, params);
    }

    // enable uart
    USART_Cmd(RS232_UART_PORT, ENABLE);
}

//------------------------------------------------------------------------------
//

#if CONFIG_RS232_PRINTF

int fputc(int ch, FILE* f)
{
    if (ch == '\n')
    {
        USART_SendData(RS232_UART_PORT, '\r');

        while (USART_GetFlagStatus(RS232_UART_PORT, USART_FLAG_TXE) == RESET)
            ;
    }

    USART_SendData(RS232_UART_PORT, (uint8_t)ch);

    while (USART_GetFlagStatus(RS232_UART_PORT, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}

#endif

#if CONFIG_RS232_SCANF

bool rs232_readable(void)
{
    return false;
}

#endif
