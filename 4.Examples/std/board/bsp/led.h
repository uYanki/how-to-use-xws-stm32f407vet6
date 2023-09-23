#ifndef __BSP_LED_H__
#define __BSP_LED_H__

#include <stdint.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
// led id

#define LED_NUM 3

#define LED1    0
#define LED2    1
#define LED3    2

//------------------------------------------------------------------------------
// led control

inline void led_on(uint8_t id);
inline void led_off(uint8_t id);
inline void led_tgl(uint8_t id);

#endif
