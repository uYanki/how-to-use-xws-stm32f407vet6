#include "main.h"
#include "dwt/dwt.h"

//  __attribute__((always_inline))
//  __attribute__((noinline))
//  __attribute__((noreturn))
//  __attribute__((used))
//  __attribute__((unused)))

//----------------------------------------------------------

#include <stdio.h>

// 可变参数宏, ## 的作用是 当 args 的参数个数为 0 时, 去除前前面的逗号
// #define println(fmt, args...) printf(fmt "\r\n", ##args)
#define println(fmt, ...) printf(fmt "\r\n", ##__VA_ARGS__)

int fputc(int ch, FILE* f)
{
    USART_SendData(RS232_UART_PORT, (uint8_t)ch);

    while (USART_GetFlagStatus(RS232_UART_PORT, USART_FLAG_TXE) == RESET)
        ;

    return (ch);
}

int fgetc(FILE* f)
{
    while (USART_GetFlagStatus(RS232_UART_PORT, USART_FLAG_RXNE) == RESET)
        ;

    return (int)USART_ReceiveData(RS232_UART_PORT);
}

//----------------------------------------------------------

// 补丁函数(闭包)， MDK - extension

void sayhello(void)
{
    printf("hello");
}

void $Sub$$sayhello(void)
{
    // MDK 拓展语法: https://zhuanlan.zhihu.com/p/145219269
    // μVision Help -> search `USE of $Sub and $Super`
    // position: Compiler Getting Started Guide. Hardware initialization.

    // before
    printf("<*>");

    // 调用原函数
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern void $Super$$sayhello(void);
    $Super$$sayhello();
#elif defined(__ICCARM__) || defined(__GNUC__)
    extern void sayhello(void);
    sayhello();
#endif

    // after
    printf("\r\n");
}

//----------------------------------------------------------

// GUN - Extension: https://blog.csdn.net/shangzh/article/details/39398577

// 类型检查

#if defined(__GNUC__)
// get minimum  value ( with type check )
#define _min(x, y) ({ const typeof(x) __x = (x); const typeof(y) __y = (y); (void) (&__x == &__y); __x < __y ? __x: __y; })
#else
#define _min(x, y) ((x) < (y) ? (x) : (y))
#endif

void get_minimum_value()
{
    println("%d", _min(12, 13));
}

//----------------------------------------------------------

// 数组元素批量赋值

void array_members_init()
{
    // array init
    u8 array[10] = {
        [0] = 10,
        [3] = 50,
#if defined(__GNUC__)
        [6 ... 8] = 100,
#endif
    };

    for (u8 i = 0; i < 10; ++i)
    {
        printf("%d ", array[i]);
    }

    println("");
}

//----------------------------------------------------------

// 范围选择

void switch_range_case()
{
    uint8_t num = 8;

    switch (num)
    {
        case 0:
            printf("zero");
            break;
#if defined(__GNUC__)
        case 1 ... 9:
            printf("range");
            break;
#endif
        default:
            printf("pass");
            break;
    }
}

int main()
{
    RS232_Init(115200);

    sayhello();
    get_minimum_value();
    array_members_init();
    switch_range_case();

    while (1)
    {
    }
}
