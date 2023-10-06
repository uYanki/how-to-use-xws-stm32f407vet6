#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include "math.h"

typedef float    f32;
typedef uint32_t u32;
typedef uint16_t u16;

bool PulseCalc(u32 clk, u32 freq, f32 duty, u16* prd, u16* psc, u16* ccr)
{
    if (freq > clk)
    {
        return false;
    }

    if (duty <= 0.f || duty >= 1.f)
    {
        return false;
    }

    u32 prescaler;

    bool similar = false;

    uint8_t i = 0;

    for (prescaler = 1; prescaler <= 65536; ++prescaler)
    {
        f32 period = (f32)clk / (f32)freq / (f32)prescaler;

        if (((u32)period == 0) || ((u32)period > 65536))
        {
            continue;
        }

        if ((period - (u32)period) > 0.5f)
        {
            continue;
        }

        // 寄存器值

        *ccr = period * duty;

        if (*ccr == 0)
        {
            continue;
        }

        *prd = period - 1;
        *psc = prescaler - 1;
        *ccr -= 1;

        // 实际频率

        u32 frq = clk / (*prd + 1) / (*psc + 1);

        printf("%d,%d,%d,%d\n", frq, *prd + 1, *psc + 1, *ccr + 1);

        // 频率相等

        if ((u32)period == period)
        {
            return true;
        }

        // 频率相似

        similar = true;
    }

    return similar;
}

int main()
{
    u16 arr[3];
    PulseCalc(84e6, 233, 0.5f, arr + 0, arr + 1, arr + 2);

    return 0;
}