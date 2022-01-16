#pragma once

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "utils/macro.h"

inline void stack_push(u8 data)
{
    write_memory(--cpu.registers.sp, data);
}

inline void stack_push_16bit(u16 data)
{
    write_memory_16bit(--cpu.registers.sp, MSB(data));
    write_memory_16bit(--cpu.registers.sp, LSB(data));
}

inline u8 stack_pop()
{
    return read_memory(cpu.registers.sp++);
}

inline u16 stack_pop_16bit()
{
    return read_memory(cpu.registers.sp++) + (cpu.registers.sp++ << 8);
}
