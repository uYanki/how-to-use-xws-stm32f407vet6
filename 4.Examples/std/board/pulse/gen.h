#ifndef __PULSE_GENERATE_H__
#define __PULSE_GENERATE_H__

#include "stm32f4xx.h"

// PA6 (TIM13_CH1)
void PulseInit(void);
bool PulseConfig(u32 freq, u16 duty);

#endif
