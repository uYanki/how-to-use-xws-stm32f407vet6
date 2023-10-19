#ifndef __ROT_ENC_H__
#define __ROT_ENC_H__

//----------------------------------------------------------
// 旋变编码器: 位置记录,速度计算

#include "stm32f4xx.h"

__packed typedef struct {
    vu32* u32EncRes;
    vs32* s32EncPos;
    vs32* s32EncTurns;
    vs32* s32UserSpdFb;
    vs64* s64UserPosFb;
    vu16* u16SpdCoeff;
    u32   u32LastTick;
    f32   f32DeltaTick;
} RotEncArgs_t;

void RotEncInit(RotEncArgs_t* args);
s32  RotEncCycle(RotEncArgs_t* args);

#endif
