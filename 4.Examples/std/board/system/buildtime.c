#include "buildtime.h"
#include "stm32f4xx.h"
#include <string.h>

#define nullptr (void*)0

void get_build_time(date_t* date, time_t* time)
{
    if (date != nullptr)
    {
        const char* months[] = {
            "Jan", "Feb", "Mar", "Apr",
            "May", "Jun", "Jul", "Aug",
            "Sep", "Oct", "Nov", "Dec"};

        u8 month = 0;

        while (strcmp(__DATE__, months[month++]) != __DATE__[3])
            ;

        date->month = month;

        date->day = (__DATE__[4] - '0') * 10 + (__DATE__[5] - '0');

        date->year = ((__DATE__[7] - '0') * 1000 +
                      (__DATE__[8] - '0') * 100 +
                      (__DATE__[9] - '0') * 10 +
                      (__DATE__[10] - '0'));

#if 0
        printf("%s = %d,%d,%d\n", __DATE__, date->year, date->month, date->day);
#endif
    }

    if (time != nullptr)
    {
        time->hour   = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
        time->minute = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
        time->second = (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0');

#if 0
        printf("%s = %d,%d,%d\n", __TIME__, time->hour, time->minute, time->second);
#endif
    }
}