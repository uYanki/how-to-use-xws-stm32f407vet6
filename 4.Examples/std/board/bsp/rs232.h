#ifndef __BSP_RS232_H__
#define __BSP_RS232_H__

#include "board_conf.h"

#if CONFIG_RS232_PRINTF
#include <stdio.h>
#endif

void rs232_init(USART_InitTypeDef* params);

#endif
