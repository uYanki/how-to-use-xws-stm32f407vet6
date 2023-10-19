#ifndef __HALL_ENC_H__
#define __HALL_ENC_H__

//----------------------------------------------------------
// 霍尔编码器: 位置记录,速度计算

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
} HallEncArgs_t;

void HallEncInit(HallEncArgs_t* args);
s32  HallEncCycle(HallEncArgs_t* args);

#endif
