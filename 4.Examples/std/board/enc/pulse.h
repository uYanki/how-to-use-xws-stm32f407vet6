#ifndef __PULSE_GEN_H__
#define __PULSE_GEN_H__

#include "stm32f4xx.h"

void PulseGenInit(void);
bool PulseGenConfig(u32 freq, u16 duty);

#endif
