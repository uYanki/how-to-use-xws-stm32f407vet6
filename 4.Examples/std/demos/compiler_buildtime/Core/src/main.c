#include "main.h"
#include "system/buildtime.h"

void usdk_hw_uart_init()
{
    RS232_Init(115200);
}

int main()
{
    usdk_hw_uart_init();

    date_time_t t;
    get_build_time(&(t.date), &(t.time));

    while (1)
    {
    }
}
