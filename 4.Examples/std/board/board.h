#ifndef __BOARD_H__
#define __BOARD_H__

#include "pinmap.h"

void Led_Init(void);
void Key_Init(void);

void RS232_Init(u32 baudrate);

void FirewareDelay(u32 nWaitTime);

#endif
