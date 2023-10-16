#ifndef __TIM_CALC_H__
#define __TIM_CALC_H__

#include "stm32f4xx.h"

bool TIM_Calc(u32 clk_i, u32 frq_i, u32 err_i, u16* prd_o, u16* psc_o, u32* frq_o);

#endif
