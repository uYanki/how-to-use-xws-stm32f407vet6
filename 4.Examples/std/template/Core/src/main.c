#include "main.h"

int main()
{
    Led_Init();
    Key_Init();

    while (1)
    {
        GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, (BitAction)GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN));
        GPIO_WriteBit(LED2_GPIO_PORT, LED2_GPIO_PIN, (BitAction)GPIO_ReadInputDataBit(KEY2_GPIO_PORT, KEY2_GPIO_PIN));
        GPIO_WriteBit(LED3_GPIO_PORT, LED3_GPIO_PIN, (BitAction)GPIO_ReadInputDataBit(KEY3_GPIO_PORT, KEY3_GPIO_PIN));
    }
}
