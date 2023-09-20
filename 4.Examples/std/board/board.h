#ifndef __BOARD_H__
#define __BOARD_H__

#include "pinmap.h"
#include "board_conf.h"

#include "stm32f4xx.h"

#if CONFIG_RS232_PRINT
#include <stdio.h>
#endif

void Led_Init(void);
void Key_Init(void);

void RS232_Init(u32 baudrate);

void FirewareDelay(u32 nWaitTime);

#endif
