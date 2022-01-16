#pragma once

#include "CPU/cpu.h"
#include "utils/types.h"

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10

inline void set_flag(u16 flag, bool value)
{
    cpu.registers.f |= flag * value;
}

inline void set_all_flags(bool z, bool n, bool h, bool c)
{
    cpu.registers.f = (z << 7) | (n << 6) | (h << 5) | (c << 4);
}

inline bool get_flag(u16 flag)
{
    return cpu.registers.f & flag;
}

inline u16 get_all_flags(u16 flag)
{
    return cpu.registers.f;
}
