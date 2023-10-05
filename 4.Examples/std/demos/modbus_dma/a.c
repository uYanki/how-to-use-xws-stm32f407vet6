#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include <string.h>

typedef float float32_t;

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

    if (decimal == NULL)
    {
        printf("%d\n", *integer);
    }
    else
    {
        printf("%f\n", *integer + *decimal);
    }

    return true;
}

#define STR(x) #x

int main()
{
    int32_t   i;
    float32_t d;

    const uint8_t* p = STR(-0b1000 #5432);

    getNumber(&p, &i, &d);
}