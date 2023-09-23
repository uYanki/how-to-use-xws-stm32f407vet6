#include "stdio.h"

int main()
{
    printf("%d", ((3UL << 10 * 2) | (3UL << 11 * 2)));
}