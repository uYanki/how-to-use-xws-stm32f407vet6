#ifndef __AUTO_INIT_H__
#define __AUTO_INIT_H__

#include "sleep.h"

#include "board_conf.h"

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
/* ARM Compiler */
/* GCC Compiler */
#define __SECTION(x) __attribute__((section(x)))
#define __USED       __attribute__((used))
#define __ALIGN(n)   __attribute__((aligned(n)))
#define __WEAK       __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
/* for IAR Compiler */
#define __SECTION(x) @x
#define __USED       __root
#define __ALIGN(n)   _Pragma("data_alignment=" #n)
#define __WEAK       __weak
#elif defined(__TI_COMPILER_VERSION__)
#define __SECTION(x)
#define __USED
#define __ALIGN(n)
#define __WEAK __attribute__((weak))
#else
#error "not supported tool chain..."
#endif

typedef int (*lpfn_init_t)(void);

// the compiler automatically sorts based on the to of ascii codes:
// small ones have higher priority, while large ones have lower priority.

#if CONFIG_USDK_INIT_DEBUG

#include <stdio.h>

typedef struct {
    lpfn_init_t func;
    const char* name;
} usdk_init_t;

#define USDK_INIT_EXPORT(func, level) \
    __SECTION(".usdk.init" level)     \
    __USED const usdk_init_t __usdk_init_##func = {func, #func};

#else

#define USDK_INIT_EXPORT(func, level) \
    __SECTION(".usdk.init" level)     \
    __USED const lpfn_init_t __usdk_init_##func = func;

#endif

#define INIT_LEVEL_BOARD       "1"  // board support package
#define INIT_LEVEL_DEVICE      "2"  // device
#define INIT_LEVEL_COMPONENT   "3"  // components
#define INIT_LEVEL_ENVIRONMENT "4"  // environment
#define INIT_LEVEL_APPLICATION "5"  // application

#define INIT_RESULT_SUCCESS    0
#define INIT_RESULT_FAILURE    1

extern void usdk_hw_uart_init(void);

#endif
