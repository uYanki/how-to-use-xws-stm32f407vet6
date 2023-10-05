#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f4xx.h"

#include "main.h"
#include "pinmap.h"
#include "system/sleep.h"

#include "bsp/led.h"
#include "bsp/key.h"
#include "bsp/uart.h"


typedef float float32_t;

#define nullptr NULL

//-----------------------------------------------------------------------------
//

void usdk_preinit(void)
{
    // sleep
    sleep_init();
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

#define INOPEN(val, lo, hi)  (((lo) < (val)) && ((val) < (hi)))    ///< check if the val is within the open range
#define INCLOSE(val, lo, hi) (((lo) <= (val)) && ((val) <= (hi)))  ///< check if the val is within the closed range

#define isBin(ch)            (INCLOSE(ch, '0', '1'))
#define isDec(ch)            (INCLOSE(ch, '0', '9'))
#define isHex(ch)            (INCLOSE(ch, '0', '9') && INCLOSE(ch, 'a', 'f') && INCLOSE(ch, 'A', 'F'))

uint8_t ascii2number(const uint8_t* buffer, uint32_t* result, uint8_t base)
{
    uint8_t limits[3];
    uint8_t length = 0;

    if (base > 36)
    {
        goto exit;
    }

    limits[0] = (base < 10) ? ('0' + base) : '9';
    limits[1] = 'a' + base - 11;
    limits[2] = 'A' + base - 11;

    *result = 0;

    while (*buffer)
    {
        if (INCLOSE(*buffer, '0', limits[0]))
        {
            *result *= base;
            *result += *buffer - '0';
        }
        else if (INCLOSE(*buffer, 'a', limits[1]))
        {
            *result *= base;
            *result += *buffer - 'a';
        }
        else if (INCLOSE(*buffer, 'A', limits[2]))
        {
            *result *= base;
            *result += *buffer - 'A';
        }
        else
        {
            break;
        }

        ++length;
        ++buffer;
    }

exit:

    return length;
}

bool getNumber(const uint8_t** buffer, int32_t* integer, float32_t* decimal)
{
    // https://www.sojson.com/hexconvert.html
    // support format (e.g): 0b101,0777,-0xABCD,6#0101,16#5A5A,-0x10#5A5A,...

    const uint8_t* ptr = *buffer;

    uint8_t sign;
    uint8_t base;
    uint8_t len;

    // 符号

    switch (*ptr)
    {
        case '-':
            ++ptr;
            sign = 0;
            break;
        case '+':
            ++ptr;
        case '0' ... '9':
            sign = 1;
            break;
        default:
            return false;
    }

    // 进制

    switch (*ptr)
    {
        default:
        case '\0': {
            return false;
        }
        case '0': {
            switch (*++ptr)
            {
                default:
                case '\0': {
                    // dec
                    *integer = 0;
                    *decimal = 0;
                    // base = 10;
                    goto exit;
                }
                case 'b':
                case 'B': {
                    // bin
                    ++ptr;
                    base = isBin(*ptr) ? 2 : 10;
                    break;
                }
                case 'x':
                case 'X': {
                    // hex
                    ++ptr;
                    base = isHex(*ptr) ? 16 : 10;
                    break;
                }
                case '0' ... '7': {
                    // oct
                    base = 8;
                    break;
                }
            }
            break;
        }
        case '1' ... '9': {
            // dec
            base = 10;
            break;
        }
    }

    // 整数部分

    {
        ptr += ascii2number(ptr, (uint32_t*)integer, base);

        if (*ptr == '#')
        {
            base = *integer;

            if ((len = ascii2number(++ptr, (uint32_t*)integer, base)) == 0)
            {
                // error format
                return false;
            }

            ptr += len;
        }

        if (sign == 0)  // neg
        {
            *integer = -*integer;
        }
    }

    // 小数部分

    {
        if (decimal != NULL && *ptr == '.')
        {
            uint32_t tmp;

            if ((len = ascii2number(++ptr, &tmp, base)) == 0)
            {
                // error format
                return false;
            }

            ptr += len;

            //

            *decimal = tmp;

            while (len--)
            {
                *decimal /= base;
            }
        }
    }

exit:

#if 0

    if (decimal == NULL)
    {
        printf("%d\n", *integer);
    }
    else
    {
        printf("%f\n", *integer + *decimal);
    }

#endif

    *buffer = ptr;

    return true;
}

#include "mbslave.h"
int main()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    u16 t = 0;

    eMBInit(MB_RTU, 0x01, 2, 115200, MB_PAR_NONE);
    // eMBInit(MB_RTU, 0x0A, 2, 115200, MB_PAR_NONE);

    // const UCHAR ucSlaveID[] = {0xAA, 0xBB, 0xCC};
    // eMBSetSlaveID(0x33, FALSE, ucSlaveID, 3);

    // test data

    usRegInputBuf[0] = 0x1234;
    usRegInputBuf[1] = 0x2345;
    usRegInputBuf[2] = 0x3456;
    usRegInputBuf[3] = 0x4567;
    usRegInputBuf[4] = 0x0000;
    usRegInputBuf[5] = 0x0000;

    usRegHoldingBuf[0] = 0x0000;
    usRegHoldingBuf[1] = 0xFFFF;
    usRegHoldingBuf[2] = 0x0000;
    usRegHoldingBuf[3] = 0xFFFF;
    usRegHoldingBuf[4] = 0x0000;
    usRegHoldingBuf[5] = 0xFFFF;
    usRegHoldingBuf[6] = 0x0000;
    usRegHoldingBuf[7] = 0xFFFF;

    ucDiscInBuf[0] = 0b11001100;
    ucDiscInBuf[1] = 0b00110011;

    ucCoilBuf[0] = 0b10101010;
    ucCoilBuf[1] = 0b01010101;
    ucCoilBuf[2] = 0b11001100;
    ucCoilBuf[3] = 0b00110011;

    eMBEnable();

    while (1)
    {
        eMBPoll();

        if (key_is_press(KEY1))
        {
            if (++t == 0)
            {
                UART_Transmit_DMA("hello\r\n", 7, false);
            }
        }
        else
        {
            t = 0;
        }
    }
}