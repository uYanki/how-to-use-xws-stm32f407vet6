#ifndef __INC_ENC_H__
#define __INC_ENC_H__

//----------------------------------------------------------
// 绝对值编码器: 位置记录,速度计算

#include "stm32f4xx.h"

__packed typedef struct {
    vu32* u32EncRes;
    vs32* s32EncPos;
    vs32* s32EncTurns;
    vs16* s16UserSpdFb;
    vs64* s64UserPosFb;
    u32   u32LastTick;
    f32   f32DeltaTick;
} AbsEncArgs_t;

void AbsEncInit(IncEncArgs_t* args);
void AbsEncCycle(IncEncArgs_t* args);

#endif
