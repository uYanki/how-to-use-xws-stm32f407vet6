#ifndef __INC_ENC_H__
#define __INC_ENC_H__

//----------------------------------------------------------
// ����������/����ʽ������: λ�ü�¼,�ٶȼ���

#include "stm32f4xx.h"

__packed typedef struct {
    vu32* u32EncRes;
    vs32* s32EncPos;
    vs32* s32EncTurns;
    vs16* s16UserSpdFb;
    vs64* s64UserPosFb;
    u32   u32LastTick;
    f32   f32DeltaTick;
} IncEncArgs_t;

void IncEncInit(IncEncArgs_t* args);
void IncEncCycle(IncEncArgs_t* args);

#endif
