#include "main.h"
#include "dwt/dwt.h"
#include "core_cm4.h"

#define __SECTION(x)  __attribute__((used, section(x)))

// https://www.cnblogs.com/svchao/p/16939949.html

// Arm Compiler 5
// u32 a  __SECTION( ".swrst_keep");//__attribute__( ( section( ".swrst_keep"), zero_init) ) ;

u32 a __attribute__(  (zero_init) ) ;

// Arm Compiler 6
// u32 NI_longVar __attribute__( ( section( ".bss.NoInit")) ) ;
//also modify one line of the scatterfile from:
// *(NoInit)
//to:
// *(.bss.NoInit)

//#define __no_init __attribute__((zero_init))



//#define __AT(x)  __attribute__((at(x)))

//   __AT(0x2001FF00) u32 a = 0;

u32 b ; 
 
int main()
{
    RS232_Init(115200);
	DWT_Init();
	++a,++b;
		printf("%x,%d\n",a,b);
	DWT_Wait(1,UNIT_S);
	NVIC_SystemReset();

    while (1)
    {
		
    }
}
