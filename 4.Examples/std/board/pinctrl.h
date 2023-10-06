#ifndef __PIN_CTRL_H__
#define __PIN_CTRL_H__

#include "stm32f4xx.h"

//-----------------------------------------------------------------------------
//

#define BITBAND(addr, bit) ((addr & 0xF0000000) + 0x02000000 + ((addr & 0x000FFFFF) << 5) + (bit << 2))
#define BITADDR(addr, bit) (*(vu32*)(BITBAND(addr, bit)))

//-----------------------------------------------------------------------------
//

#define GPIOA_ODR_Addr     (GPIOA_BASE + 20)
#define GPIOB_ODR_Addr     (GPIOB_BASE + 20)
#define GPIOC_ODR_Addr     (GPIOC_BASE + 20)
#define GPIOD_ODR_Addr     (GPIOD_BASE + 20)
#define GPIOE_ODR_Addr     (GPIOE_BASE + 20)
#define GPIOF_ODR_Addr     (GPIOF_BASE + 20)
#define GPIOG_ODR_Addr     (GPIOG_BASE + 20)
#define GPIOH_ODR_Addr     (GPIOH_BASE + 20)
#define GPIOI_ODR_Addr     (GPIOI_BASE + 20)
#define GPIOJ_ODR_Addr     (GPIOJ_BASE + 20)
#define GPIOK_ODR_Addr     (GPIOK_BASE + 20)

#define GPIOA_IDR_Addr     (GPIOA_BASE + 16)
#define GPIOB_IDR_Addr     (GPIOB_BASE + 16)
#define GPIOC_IDR_Addr     (GPIOC_BASE + 16)
#define GPIOD_IDR_Addr     (GPIOD_BASE + 16)
#define GPIOE_IDR_Addr     (GPIOE_BASE + 16)
#define GPIOF_IDR_Addr     (GPIOF_BASE + 16)
#define GPIOG_IDR_Addr     (GPIOG_BASE + 16)
#define GPIOH_IDR_Addr     (GPIOH_BASE + 16)
#define GPIOI_IDR_Addr     (GPIOI_BASE + 16)
#define GPIOJ_IDR_Addr     (GPIOJ_BASE + 16)
#define GPIOK_IDR_Addr     (GPIOK_BASE + 16)

//-----------------------------------------------------------------------------
//

#define PAout(n)           BITADDR(GPIOA_ODR_Addr, n)
#define PAin(n)            BITADDR(GPIOA_IDR_Addr, n)

#define PBout(n)           BITADDR(GPIOB_ODR_Addr, n)
#define PBin(n)            BITADDR(GPIOB_IDR_Addr, n)

#define PCout(n)           BITADDR(GPIOC_ODR_Addr, n)
#define PCin(n)            BITADDR(GPIOC_IDR_Addr, n)

#define PDout(n)           BITADDR(GPIOD_ODR_Addr, n)
#define PDin(n)            BITADDR(GPIOD_IDR_Addr, n)

#define PEout(n)           BITADDR(GPIOE_ODR_Addr, n)
#define PEin(n)            BITADDR(GPIOE_IDR_Addr, n)

#define PFout(n)           BITADDR(GPIOF_ODR_Addr, n)
#define PFin(n)            BITADDR(GPIOF_IDR_Addr, n)

#define PGout(n)           BITADDR(GPIOG_ODR_Addr, n)
#define PGin(n)            BITADDR(GPIOG_IDR_Addr, n)

#define PHout(n)           BITADDR(GPIOH_ODR_Addr, n)
#define PHin(n)            BITADDR(GPIOH_IDR_Addr, n)

#define PIout(n)           BITADDR(GPIOI_ODR_Addr, n)
#define PIin(n)            BITADDR(GPIOI_IDR_Addr, n)

#define PJout(n)           BITADDR(GPIOJ_ODR_Addr, n)
#define PJin(n)            BITADDR(GPIOJ_IDR_Addr, n)

#define PKout(n)           BITADDR(GPIOK_ODR_Addr, n)
#define PKin(n)            BITADDR(GPIOK_IDR_Addr, n)

#endif
