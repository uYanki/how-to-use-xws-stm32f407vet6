#include "main.h"
#include "system/sleep.h"

#include <stdio.h>

int main()
{
    sleep_init();
    RS232_Init(115200);

    while (1)
    {
        printf("hello stm32\n");
        sleep_s(1);
    }
}
