#pragma once

#include "cpu/cpu.h"
#include "cpu/memory.h"
#include "cpu/timer.h"
#include "utils/macro.h"

ALWAYS_INLINE void stack_push(u8 data)
{
    timer_tick();
    write_memory(--g_cpu.registers.sp, data);
}

ALWAYS_INLINE void stack_push_16bit(u16 data)
{
    stack_push(MSB(data));
    stack_push(LSB(data));
}

ALWAYS_INLINE u8 stack_pop()
{
    timer_tick();
    return read_memory((g_cpu.registers.sp)++);
}

ALWAYS_INLINE u16 stack_pop_16bit()
{
    const u8 lsb = stack_pop();
    const u8 msb = stack_pop();
    return lsb | (msb << 8);
}
