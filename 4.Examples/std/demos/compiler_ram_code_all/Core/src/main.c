#include "main.h"
#include "dwt/dwt.h"

void DMA1_Stream1_IRQHandler(void){}

int main()
{
    RS232_Init(115200);
	

	printf("0x%p\n",DMA1_Stream1_IRQHandler);

    while (1)
    {
    }
}
