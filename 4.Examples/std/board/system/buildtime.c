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

        const char* _date = __DATE__;

        u8 month = 0;

        while (strcmp(_date, months[month++]) != _date[3])
            ;

        date->month = month;

        date->day = (_date[4] - '0') * 10 + (_date[5] - '0');

        date->year = ((_date[7] - '0') * 1000 +
                      (_date[8] - '0') * 100 +
                      (_date[9] - '0') * 10 +
                      (_date[10] - '0'));

#if 0
        printf("%s = %04d,%02d,%02d\n", _date, date->year, date->month, date->day);
#endif
    }

    if (time != nullptr)
    {
        const char* _time = __TIME__;

        time->hour   = (_time[0] - '0') * 10 + (_time[1] - '0');
        time->minute = (_time[3] - '0') * 10 + (_time[4] - '0');
        time->second = (_time[6] - '0') * 10 + (_time[7] - '0');

#if 0
        printf("%s = %02d,%02d,%02d\n", _time, time->hour, time->minute, time->second);
#endif
    }
}
