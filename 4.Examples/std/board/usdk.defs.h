#ifndef __USKD_DEFS_H__
#define __USKD_DEFS_H__

#define nullptr                (void*)0

//

#define S8_MIN                 (s8)0x80                 ///< -128
#define S16_MIN                (s16)0x8000              ///< -32678
#define S32_MIN                (s32)0x80000000          ///< -2147483648
#define S64_MIN                (s64)0x8000000000000000  ///< -9223372036854775808

#define S8_MAX                 (s8)0x7F                 ///< +127
#define S16_MAX                (s16)0x7FFF              ///< +32767
#define S32_MAX                (s32)0x7FFFFFFF          ///< +2147483647
#define S64_MAX                (s64)0x7FFFFFFFFFFFFFFF  ///< +9223372036854775807

#define U8_MAX                 (u8)0xFF                 ///< +255
#define U16_MAX                (u16)0xFFFF              ///< +65535
#define U32_MAX                (u32)0xFFFFFFFF          ///< +4294967295
#define U64_MAX                (u64)0xFFFFFFFFFFFFFFFF  ///< +18446744073709551615

//

#define REG8S(addr)            (*(s8*)(addr))
#define REG8U(addr)            (*(u8*)(addr))

#define REG16S(addr)           (*(s16*)(addr))
#define REG16U(addr)           (*(u16*)(addr))
#define REG32S(addr)           (*(s32*)(addr))
#define REG32U(addr)           (*(u32*)(addr))
#define REG64S(addr)           (*(s64*)(addr))
#define REG64U(addr)           (*(u64*)(addr))

//

#define BV(n)                  (1u << (n))

#define MASK8(LEN)             (~(U8_MAX << (LEN)))
#define MASK16(LEN)            (~(U16_MAX << (LEN)))
#define MASK32(LEN)            (~(U32_MAX << (LEN)))
#define MASK64(LEN)            (~(U64_MAX << (LEN)))

// @note STB: start bit

#define SETBIT(DAT, BIT)       ((DAT) |= (1UL << (BIT)))           ///< set a bit (bit-32)
#define CLRBIT(DAT, BIT)       ((DAT) &= (~(1UL << (BIT))))        ///< clear a bit (bit-32)
#define INVBIT(DAT, BIT)       ((DAT) ^= (1UL << (BIT)))           ///< invert a bit (bit-32)
#define GETBIT(DAT, BIT)       (((DAT) >> (BIT)) & 0x1)            ///< get a bit (bit-32/64)
#define SETBITS(DAT, STB, LEN) ((DAT) |= (MASK32(LEN) << (STB)))   ///< set bits (bit-32)
#define CLRBITS(DAT, STB, LEN) ((DAT) &= ~(MASK32(LEN) << (STB)))  ///< clear bits (bit-32)
#define INVBITS(DAT, STB, LEN) ((DAT) ^= (MASK32(LEN) << (STB)))   ///< invert bits (bit-32)
#define GETBITS(DAT, STB, LEN) (((DAT) >> (STB)) & MASK32(LEN))    ///< get bits (bit-32)

#define SETBIT64(DAT, BIT)     ((DAT) |= (1ULL << (BIT)))     ///< set a bit (bit-64)
#define CLRBIT64(DAT, BIT)     ((DAT) &= (~(1ULL << (BIT))))  ///< clear a bit (bit-64)
#define INVBIT64(DAT, BIT)     ((DAT) ^= (1ULL << (BIT)))     ///< invert a bit (bit-64)

#endif
