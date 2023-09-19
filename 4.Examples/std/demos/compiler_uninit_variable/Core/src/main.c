#include "main.h"
#include "dwt/dwt.h"
#include "core_cm4.h"

#if (__ARMCC_VERSION >= 6000000)  // Arm Compiler 6
#define __NOINIT __attribute__((section(".bss.ARM.__at_0x2001FF00")))
#elif (__ARMCC_VERSION >= 5000000)  // Arm Compiler 5
// #define __NOINIT __attribute__((at(0x2001FF00)));
#define __NOINIT  __attribute__((zero_init, used, section(".swrst_keep")))
#else
#error "unsupported version"
#endif

u32 a __NOINIT;
u32 b;

int main()
{
    DWT_Init();
    RS232_Init(115200);

    ++a, ++b;
    printf("%x,%d\n", a, b);

    DWT_Wait(1, UNIT_S);
    NVIC_SystemReset();  // rst

    while (1)
    {
    }
}
