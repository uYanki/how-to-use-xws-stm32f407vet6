#ifndef __BUILD_TIME_H__
#define __BUILD_TIME_H__

#include "stm32f4xx.h"

#define CONFIG_VERSION_MAJOR 5  // Major
#define CONFIG_VERSION_MINOR 0  // Minor
#define CONFIG_VERSION_PATCH 1  // Patch

#define USDK_VERSION         ((CONFIG_VERSION_MAJOR * 10000) + (CONFIG_VERSION_MINOR * 100) + (CONFIG_VERSION_PATCH))

typedef struct {
    u16 year;
    u8  month;
    u8  day;
} date_t;

typedef struct {
    u8 hour;
    u8 minute;
    u8 second;
} time_t;

typedef struct {
    date_t date;
    time_t time;
} date_time_t;

void get_build_time(date_t* date, time_t* time);

// examples:
//
//  date_time_t buildTime;
//  get_build_time(&(buildTime.date), &(buildTime.time));
//

#endif
