#ifndef __PULSE_H__
#define __PULSE_H__

#include "stm32f4xx.h"

// PA6 (TIM13_CH1)
void PulseGenInit(void);
bool PulseGenConfig(u32 freq, u16 duty);

#endif
