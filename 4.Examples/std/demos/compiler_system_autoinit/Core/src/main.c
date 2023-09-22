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

    // �ϵ��ӡ�Ĵ���ֵ
    printf("CSR = 0x%X\n", RCC->CSR);

    if (RCC->CSR & BV(31))
    {  // 1.�͹��Ĺ���λ
        printf("1.Low-power reset\n");
    }
    if (RCC->CSR & BV(30))
    {  // 2.���ڿ��Ź���λ
        printf("2.Window watchdog reset\n");
    }
    if (RCC->CSR & BV(29))
    {  // 3.�������Ź���λ
        printf("3.(Independent watchdog reset\n");
    }
    if (RCC->CSR & BV(28))
    {  // 4.�����λ
        printf("4.Software reset\n");
    }
    if (RCC->CSR & BV(27))
    {  // 5.�ϵ�/���縴λ
        printf("5.POR/PDR reset\n");
    }
    if (RCC->CSR & BV(26))
    {  // 6.PIN���Ÿ�λ
        printf("6.PIN reset\n");
    }

    // �����λ��־
    RCC->CSR = 0x01000000;

    printf("--------\n");

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(display_reset_reason, INIT_LEVEL_DEVICE)
