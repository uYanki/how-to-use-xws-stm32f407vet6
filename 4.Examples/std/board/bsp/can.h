#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__

#include <stdint.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
// can control

bool can_init(CAN_TypeDef* CANx, u32 bitrate);
void can_display_msg(const char* tag, void* msg);

#endif
