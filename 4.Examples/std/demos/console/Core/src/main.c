#include "main.h"
#include "system/sleep.h"
#include "bsp/uart.h"

#include "pinctrl.h"
#include "pinmap.h"

//-----------------------------------------------------------------------------
//

void usdk_preinit(void)
{
    // sleep
    sleep_init();
    // delay
    DelayInit();
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

//

int main()
{

    while (1)
    {


    }
}
