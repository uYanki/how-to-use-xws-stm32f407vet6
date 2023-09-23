#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include <stdint.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
// key id

#define KEY_NUM 3

#define KEY1    0
#define KEY2    1
#define KEY3    2

//------------------------------------------------------------------------------
// key state

inline bool key_is_press(uint8_t id);
inline bool key_is_release(uint8_t id);

#endif
