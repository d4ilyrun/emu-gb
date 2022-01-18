#pragma once

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "utils/macro.h"

ALWAYS_INLINE void stack_push(u8 data)
{
    write_memory(--cpu.registers.sp, data);
}

ALWAYS_INLINE void stack_push_16bit(u16 data)
{
    write_memory_16bit(--cpu.registers.sp, MSB(data));
    write_memory_16bit(--cpu.registers.sp, LSB(data));
}

ALWAYS_INLINE u8 stack_pop()
{
    return read_memory((cpu.registers.sp)++);
}

ALWAYS_INLINE u16 stack_pop_16bit()
{
    u16 sp = read_register_16bit(REG_SP);
    write_register_16bit(REG_SP, sp + 2);
    return read_memory(cpu.registers.sp) + ((cpu.registers.sp + 1) << 8);
}
