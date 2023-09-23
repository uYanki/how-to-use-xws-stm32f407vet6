#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "system/autoinit.h"

#include "pinmap.h"
#include "key.h"

//------------------------------------------------------------------------------
//

static int key_init(void)
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

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(key_init, INIT_LEVEL_BOARD)

//------------------------------------------------------------------------------
//

bool key_is_press(uint8_t id)
{
    switch (id)
    {
        case KEY1: return GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == Bit_RESET;
        case KEY2: return GPIO_ReadInputDataBit(KEY2_GPIO_PORT, KEY2_GPIO_PIN) == Bit_RESET;
        case KEY3: return GPIO_ReadInputDataBit(KEY3_GPIO_PORT, KEY3_GPIO_PIN) == Bit_RESET;
        default: return false;
    }
}

bool key_is_release(uint8_t id)
{
    return !key_is_press(id);
}
