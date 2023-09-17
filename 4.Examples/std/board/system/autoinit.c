#include "autoinit.h"

static int system_begin()
{
    return 0;
}

USDK_INIT_EXPORT(system_begin, INIT_LEVEL_FIRST)  // first

static int system_end()
{
    return 0;
}

USDK_INIT_EXPORT(system_end, INIT_LEVEL_LAST)  // last

void usdk_startup()
{
    extern void usdk_hw_uart_init(void);
    usdk_hw_uart_init();

#if CONFIG_USDK_INIT_DEBUG

    volatile const usdk_init_t* ptr;

    for (ptr = &__usdk_init_system_begin; ptr <= &__usdk_init_system_end; ptr++)
    {
        printf("%s: %d\n", ptr->name, (ptr->func)());
    }

#else

    volatile const lpfn_init_t* ptr;

    for (ptr = &__usdk_init_begin; ptr <= &__usdk_init_end; ptr++)
    {
        (*ptr)();
    }

#endif
}

//

#if defined(__ARMCC_VERSION)
/* re-define main function */
int $Sub$$main(void)
{
    extern int $Super$$main(void);
    usdk_startup();
    return $Super$$main(); /* for ARMCC. */
}
#elif defined(__ICCARM__)
/* __low_level_init will auto called by IAR cstartup */
int __low_level_init(void)
{
    extern void __iar_data_init3(void);
    // call IAR table copy function.
    __iar_data_init3();
    usdk_startup();
    return 0;
}
#elif defined(__GNUC__)
/* Add -eentry to arm-none-eabi-gcc argument */
int entry(void)
{
    extern int main(void);
    usdk_startup();
    return main();
}
#elif defined(__TI_COMPILER_VERSION__)
#pragma INIT_SECTION(usdk_startup)
#endif

__weak void usdk_hw_uart_init()
{
}
