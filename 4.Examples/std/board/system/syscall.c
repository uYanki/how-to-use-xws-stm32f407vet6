// clang-format off

#ifndef NDEBUG

#include <stdlib.h>
#include <stdio.h>

int fputs(const char* str, FILE* stream)
{
    printf(str);
    return 0;

    // if error
    // return EOF;
}

__attribute__((weak, noreturn)) 
void __aeabi_assert(const char* expr, const char* file, int line)
{
    char str[12], *p;

    fputs("*** assertion failed: ", stderr);
    fputs(expr, stderr);
    fputs(", file ", stderr);
    fputs(file, stderr);
    fputs(", line ", stderr);
    
    p    = str + sizeof(str);
    *--p = '\0';
    *--p = '\n';
    while (line > 0)
    {
        *--p = '0' + (line % 10);
        line /= 10;
    }
    fputs(p, stderr);
    
    abort();
}

__attribute__((weak))
void abort(void)
{
    for (;;)
        ;
}

#endif

// clang-format on