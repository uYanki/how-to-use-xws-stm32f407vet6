
/* ----------------------- System includes --------------------------------- */
#include "port.h"

/* ----------------------- Modbus includes --------------------------------- */
#include "mb.h"
#include "mbport.h"
#include "mbrtu.h"

/* ----------------------- Defines ----------------------------------------- */

/* ----------------------- Static variables -------------------------------- */

/* ----------------------- Start implementation ---------------------------- */

#include "stm32f4xx.h"
#include "bsp/uart.h"
#include "string.h"

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable)
    {
        // RS485_SetRxDir();
    }

    if (xTxEnable)
    {
        // RS485_SetTxDir();
    }
}

BOOL xMBPortSerialInit(UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    // UART_HandleTypeDef* huart;

    // ucPort is set by eMBInit

    USART_InitTypeDef USART_InitStructure;

    if (ucPort == 1)
    {
        USART_InitStructure.USART_BaudRate            = ulBaudRate;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;

        switch (eParity)
        {
            case MB_PAR_NONE: USART_InitStructure.USART_Parity = USART_Parity_No; break;
            case MB_PAR_EVEN: USART_InitStructure.USART_Parity = USART_Parity_Even; break;
            case MB_PAR_ODD: USART_InitStructure.USART_Parity = USART_Parity_Odd; break;
            default: return FALSE;
        }

        if (eParity == MB_PAR_NONE)
        {
            switch (ucDataBits)
            {
                case 7: return FALSE;
                case 8: USART_InitStructure.USART_WordLength = USART_WordLength_8b; break;
            }
        }
        else
        {
            // 在 STM32 中, 数据位包含了校验位

            switch (ucDataBits)
            {
                case 7: USART_InitStructure.USART_WordLength = USART_WordLength_8b; break;
                case 8: USART_InitStructure.USART_WordLength = USART_WordLength_9b; break;
            }
        }

        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    }
    else
    {
        return FALSE;
    }

    __disable_irq();
    UART_Config(&USART_InitStructure);
    __enable_irq();

    return TRUE;
}
