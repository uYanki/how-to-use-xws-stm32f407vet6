#ifndef __ENC_AB_H__
#define __ENC_AB_H__

//----------------------------------------------------------
// 正交编码器/增量式编码器: 位置记录,速度计算 
// ( position recording, speed calculation )
//----------------------------------------------------------

#include "stm32f4xx.h"

void EncInit(void);
void EncCycle(void);

#endif
