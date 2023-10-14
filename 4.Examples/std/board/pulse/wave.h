#ifndef __WAVE_GENERATE_H__
#define __WAVE_GENERATE_H__

#include "usdk.defs.h"
#include "paratbl/tbl.h"

#define Hz_2_mHz 1000
#define mHz_2_Hz 0.001

void WaveGenInit(void);
void WaveGenCycle(void);

#endif
