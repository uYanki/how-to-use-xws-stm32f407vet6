#include "board.h"

// clang-format off
// clang-format on

void Led_Init(void)
{
    RCC_AHB1PeriphClockCmd(LED1_GPIO_CLK | LED2_GPIO_CLK | LED3_GPIO_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStruct.GPIO_Pin = LED1_GPIO_PIN;
    GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = LED2_GPIO_PIN;
    GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = LED3_GPIO_PIN;
    GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStruct);

    LED_OFF(LED1_GPIO_PORT, LED1_GPIO_PIN);
    LED_OFF(LED2_GPIO_PORT, LED2_GPIO_PIN);
    LED_OFF(LED3_GPIO_PORT, LED3_GPIO_PIN);
}

void Key_Init(void)
{
    RCC_AHB1PeriphClockCmd(KEY1_GPIO_CLK | KEY2_GPIO_CLK | KEY3_GPIO_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStruct.GPIO_Pin = KEY1_GPIO_PIN;
    GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = KEY2_GPIO_PIN;
    GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = KEY3_GPIO_PIN;
    GPIO_Init(KEY3_GPIO_PORT, &GPIO_InitStruct);
}

void RS232_Init(u32 baudrate)
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
        USART_InitTypeDef USART_InitStructure;

        RS232_UART_CLKEN(RS232_UART_CLK, ENABLE);

        USART_InitStructure.USART_BaudRate            = baudrate;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
        USART_InitStructure.USART_Parity              = USART_Parity_No;
        USART_InitStructure.USART_StopBits            = USART_StopBits_1;
        USART_InitStructure.USART_WordLength          = USART_WordLength_8b;

        USART_Init(RS232_UART_PORT, &USART_InitStructure);
    }

    // enable uart
    USART_Cmd(RS232_UART_PORT, ENABLE);
}

#if CONFIG_RS232_PRINT

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

void FirewareDelay(u32 nWaitTime)
{
    u8 n;
    while (nWaitTime--)
    {
        n = UINT8_MAX;
        while (n--)
        {
        }
    }
}
