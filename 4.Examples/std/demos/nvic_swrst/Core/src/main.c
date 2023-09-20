#include "main.h"
#include "system/sleep.h"
#include "system/swrst.h"

static int n = 0;

void usdk_hw_uart_init()
{
    RS232_Init(115200);
}

int main()
{
    sleep_init();

    usdk_hw_uart_init();

    // 上电打印寄存器值
    printf("\n\nCSR = 0x%X\n", RCC->CSR);

    if (RCC->CSR & 0x80000000)
    {  // 1.低功耗管理复位
        printf("1.Low-power reset\n");
    }
    if (RCC->CSR & 0x40000000)
    {  // 2.窗口看门狗复位
        printf("2.Window watchdog reset\n");
    }
    if (RCC->CSR & 0x20000000)
    {  // 3.独立看门狗复位
        printf("3.(Independent watchdog reset\n");
    }
    if (RCC->CSR & 0x10000000)
    {  // 4.软件复位
        printf("4.Software reset\n");
    }
    if (RCC->CSR & 0x08000000)
    {  // 5.上电/掉电复位
        printf("5.POR/PDR reset\n");
    }
    if (RCC->CSR & 0x04000000)
    {  // 6.PIN引脚复位
        printf("6.PIN reset\n");
    }

    // 清除复位标志
    RCC->CSR = 0x01000000;

		// get clock state
    printf("<1> clock (AHB1) = %d\n", RCC->AHB1ENR);
    Key_Init();
    printf("<2> clock (AHB1) = %d\n", RCC->AHB1ENR);

    // variable is reset
    printf("<3> counter = %d\n", ++n);

    sleep_s(1);

    while (1)
    {
        if (KEY_IS_PRESS(KEY1_GPIO_PORT, KEY1_GPIO_PIN))
        {
            // 复位内核, 不影响外设 (RCC->AHB1ENR 内容保持不变)

            // NVIC_CoreReset_C();
            NVIC_CoreReset_ASM();
        }

        if (KEY_IS_PRESS(KEY2_GPIO_PORT, KEY2_GPIO_PIN))
        {
            // 复位整个芯片 (RCC->AHB1ENR 被恢复为默认值)

            // NVIC_SystemReset();
            // NVIC_SystemReset_C();
            NVIC_SystemReset_ASM();
        }
    }
}
