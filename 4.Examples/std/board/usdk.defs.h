#ifndef __USKD_DEFS_H__
#define __USKD_DEFS_H__

#define nullptr      (void*)0

#define BV(n)        (1u << (n))

#define REG8S(addr)  (*(s8*)(addr))
#define REG8U(addr)  (*(u8*)(addr))

#define REG16S(addr) (*(s16*)(addr))
#define REG16U(addr) (*(u16*)(addr))

#define REG32S(addr) (*(s32*)(addr))
#define REG32U(addr) (*(u32*)(addr))

#define REG64S(addr) (*(s64*)(addr))
#define REG64U(addr) (*(u64*)(addr))

#endif
