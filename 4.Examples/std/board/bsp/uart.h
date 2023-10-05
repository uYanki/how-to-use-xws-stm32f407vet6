#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "board_conf.h"

#if CONFIG_UART_REDIRECT_PRINTF
#include <stdio.h>
#endif

void UART_Config(USART_InitTypeDef* config);
void UART_DMA_Config(void);
bool UART_Transmit_DMA(void* datsrc, u16 len, bool block /*阻塞式等待*/);

#endif
