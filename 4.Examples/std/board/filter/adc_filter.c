#include "adc_filter.h"

u16 average_filter(avg_fltr_t* filter, u16 new_value)
{
    const u16 length = filter->length;

    u16* buffer = filter->buffer;
    u16  index  = filter->index;
    u64  sum    = filter->sum;

    if (++index == length)
    {
        index = 1;
    }

    sum -= buffer[index];
    sum += new_value;

    buffer[index] = new_value;

    filter->index = index;
    filter->sum   = sum;

    return sum / length;
}