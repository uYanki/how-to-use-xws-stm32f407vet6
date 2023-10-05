
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

extern volatile UCHAR* ucRTUBuf;
extern volatile USHORT usSndBufferCount;
extern volatile USHORT usRcvBufferPos;

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xRxEnable)
    {
        // call xMBRTUReceiveFSM()
        // eRcvState: STATE_RX_INIT -> STATE_RX_RCV
        pxMBFrameCBByteReceived();

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
    //    if (ucPort == 2) {
    //        huart = &huart2;
    //    } else {
    //        return FALSE;
    //    }

    //    huart->Init.BaudRate = ulBaudRate;

    //    switch (eParity) {
    //        case MB_PAR_NONE: huart->Init.Parity = UART_PARITY_NONE; break;
    //        case MB_PAR_EVEN: huart->Init.Parity = UART_PARITY_EVEN; break;
    //        case MB_PAR_ODD: huart->Init.Parity = UART_PARITY_ODD; break;
    //        default: return FALSE;
    //    }
    //
    //		huart->Init.WordLength = UART_WORDLENGTH_8B;

    //    switch (ucDataBits) {
    //        case 8:
    //            huart->Init.WordLength = (eParity == UART_PARITY_NONE) ? UART_WORDLENGTH_8B : UART_WORDLENGTH_9B;
    //            break;
    //        case 7:
    //            if (eParity == UART_PARITY_NONE) return FALSE;
    //            huart->Init.WordLength = UART_WORDLENGTH_8B;
    //            break;
    //        default: return FALSE;
    //    }

    return TRUE;
}
