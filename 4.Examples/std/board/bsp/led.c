#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "system/autoinit.h"

#include "pinmap.h"
#include "led.h"

//------------------------------------------------------------------------------
//

static int led_init(void)
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

    led_off(LED1);
    led_off(LED2);
    led_off(LED3);

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(led_init, INIT_LEVEL_BOARD)

//------------------------------------------------------------------------------
//

void led_on(u8 id)
{
    switch (id)
    {
        case LED1: GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, Bit_RESET); break;
        case LED2: GPIO_WriteBit(LED2_GPIO_PORT, LED2_GPIO_PIN, Bit_RESET); break;
        case LED3: GPIO_WriteBit(LED3_GPIO_PORT, LED3_GPIO_PIN, Bit_RESET); break;
        default: break;
    }
}

void led_off(u8 id)
{
    switch (id)
    {
        case LED1: GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, Bit_SET); break;
        case LED2: GPIO_WriteBit(LED2_GPIO_PORT, LED2_GPIO_PIN, Bit_SET); break;
        case LED3: GPIO_WriteBit(LED3_GPIO_PORT, LED3_GPIO_PIN, Bit_SET); break;
        default: break;
    }
}

// toggle
void led_tgl(u8 id)
{
    switch (id)
    {
        case LED1: GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, (BitAction)!GPIO_ReadInputDataBit(LED1_GPIO_PORT, LED1_GPIO_PIN)); break;
        case LED2: GPIO_WriteBit(LED2_GPIO_PORT, LED2_GPIO_PIN, (BitAction)!GPIO_ReadInputDataBit(LED2_GPIO_PORT, LED2_GPIO_PIN)); break;
        case LED3: GPIO_WriteBit(LED3_GPIO_PORT, LED3_GPIO_PIN, (BitAction)!GPIO_ReadInputDataBit(LED3_GPIO_PORT, LED3_GPIO_PIN)); break;
        default: break;
    }
}
