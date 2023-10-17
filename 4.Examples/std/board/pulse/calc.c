#include "calc.h"

// err_i: maximum allowable frequency error

bool TIM_Calc(u32 clk_i, u32 frq_i, u32 err_i, u16* prd_o, u16* psc_o, u32* frq_o)
{
    if (frq_i == 0)
    {
        return false;
    }

#if 1
    if (frq_i > clk_i)
#else
    // because period >= 2
    if (frq_i > (clk_i >> 1))
#endif
    {
        return false;
    }

    bool similar = false;
    u32  prd, psc, frq;
    u32  err_pre = err_i, err_cur;

    // 周期从高往低, 便于设置低频信号的占空比
    for (prd = 65536u; prd >= 2; --prd)
    {
        psc = (f32)clk_i / (f32)frq_i / (f32)prd;

        if (psc == 0 || psc > 65536)
        {
            continue;
        }

        // 实际频率

        frq = (f32)clk_i / (f32)psc / (f32)prd;

        if (frq > clk_i)
        {
            continue;
        }

        // 频率误差

        if (frq > frq_i)
        {
            err_cur = frq - frq_i;
        }
        else
        {
            err_cur = frq_i - frq;
        }

        if (err_pre > err_cur)
        {
            err_pre = err_cur;

            if (frq_o != nullptr)
            {
                *frq_o = frq;
            }

            *prd_o = prd - 1;
            *psc_o = psc - 1;

            // 频率相等

            if (err_cur == 0)
            {
                return true;
            }

            // 频率相近

            similar = true;
        }
    }

    return similar;
}
