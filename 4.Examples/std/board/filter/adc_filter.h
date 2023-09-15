#ifndef __ADC_FILTER_H__
#define __ADC_FILTER_H__

#include "stm32f4xx.h"

typedef unsigned long long uint64_t;

typedef uint64_t u64;

typedef struct {
    u16* buffer;
    u16  length;
    u16  index;
    u64  sum;
} avg_fltr_t;

#endif