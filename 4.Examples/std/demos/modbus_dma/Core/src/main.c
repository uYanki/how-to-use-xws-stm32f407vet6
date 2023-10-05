#include <stdio.h>
#include <stdbool.h>

#include "stm32f4xx.h"

#include "main.h"
#include "pinmap.h"
#include "system/sleep.h"

#include "bsp/led.h"
#include "bsp/key.h"

//-----------------------------------------------------------------------------
//

void usdk_preinit(void)
{
    // sleep
    sleep_init();
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

int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    u16 t = 0;

    while (1)
    {
        if (key_is_press(KEY1))
        {
            if (++t == 0)
            {
                UART_Transmit_DMA("hello\r\n", 7, false);
            }
        }
        else
        {
            t = 0;
        }
    }
}
