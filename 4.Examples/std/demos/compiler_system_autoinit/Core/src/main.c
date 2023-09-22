#include "main.h"

#include <stdio.h>

#include "system/autoinit.h"
#include "system/swrst.h"

#define BV(n) (1u << (n))

void usdk_hw_uart_init()
{
    RS232_Init(115200);
}

int main()
{
    printf("\n");
		
		sleep_s(1);

    while (1)
    {
        GPIO_WriteBit(__LED1, (BitAction)GPIO_ReadInputDataBit(__KEY1));

        if (__KEY_IS_PRESS(__KEY2))
        { 
            NVIC_CoreReset_C();
        }
        if (__KEY_IS_PRESS(__KEY3))
        {
            NVIC_SystemReset_C();
        }
    }
}

int bsp_init(void)
{
    Led_Init();
    Key_Init();

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(bsp_init, INIT_LEVEL_BOARD)

int display_reset_reason(void)
{
    printf("--------\n");

    // 上电打印寄存器值
    printf("CSR = 0x%X\n", RCC->CSR);

    if (RCC->CSR & BV(31))
    {  // 1.低功耗管理复位
        printf("1.Low-power reset\n");
    }
    if (RCC->CSR & BV(30))
    {  // 2.窗口看门狗复位
        printf("2.Window watchdog reset\n");
    }
    if (RCC->CSR & BV(29))
    {  // 3.独立看门狗复位
        printf("3.(Independent watchdog reset\n");
    }
    if (RCC->CSR & BV(28))
    {  // 4.软件复位
        printf("4.Software reset\n");
    }
    if (RCC->CSR & BV(27))
    {  // 5.上电/掉电复位
        printf("5.POR/PDR reset\n");
    }
    if (RCC->CSR & BV(26))
    {  // 6.PIN引脚复位
        printf("6.PIN reset\n");
    }

    // 清除复位标志
    RCC->CSR = 0x01000000;

    printf("--------\n");

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(display_reset_reason, INIT_LEVEL_DEVICE)
