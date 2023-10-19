#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "stm32f4xx.h"

void AdcInit(void);
void AdcOvsCalc(vu16* pau16Output);

#endif
