#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stm32f4xx.h"

void ds18b20_init(void);
bool ds18b20_reset(void);
bool ds18b20_read_id(u8* id);
bool ds18b20_read_temp(s16* temp);
bool ds18b20_read_temp_by_id(u8 id[8], s16* temp);
bool ds18b20_read_temp_crc(s16* temp);
bool ds18b20_set_resolution(u8 res);
f32  ds18b20_convert_temp(s16 temp);

#endif
