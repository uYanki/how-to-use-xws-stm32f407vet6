#include "main.h"
#include "dwt/dwt.h"

#include <stdio.h>

int main()
{
    RS232_Init(115200);

    DWT_Init();

    while (1)
    {
        printf("hello stm32\n");
        DWT_Wait(1, UNIT_S);
    }
}
