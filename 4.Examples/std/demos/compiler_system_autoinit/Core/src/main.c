#include "main.h"

#include <stdio.h>

#include "system/autoinit.h"

//-----------------------------------
//

void usdk_hw_uart_init()
{
    RS232_Init(115200);
}

//-----------------------------------
//

int bsp_init(void)
{
    Led_Init();
    Key_Init();

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(bsp_init, INIT_LEVEL_BOARD)

int main()
{
    while (1)
    {
        GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, (BitAction)GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN));
    }
}
