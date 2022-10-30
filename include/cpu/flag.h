#pragma once

#include "cpu/cpu.h"
#include "utils/macro.h"
#include "utils/types.h"

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

ALWAYS_INLINE void set_flag(u16 flag, bool value)
{
    if (value)
        g_cpu.registers.f |= flag;
    else
        g_cpu.registers.f &= ~flag;
}

ALWAYS_INLINE void set_all_flags(bool z, bool n, bool h, bool c)
{
    g_cpu.registers.f = (z << 7) | (n << 6) | (h << 5) | (c << 4);
}

ALWAYS_INLINE bool get_flag(u16 flag)
{
    return g_cpu.registers.f & flag ? 1 : 0;
}

ALWAYS_INLINE u16 get_all_flags()
{
    return g_cpu.registers.f;
}
