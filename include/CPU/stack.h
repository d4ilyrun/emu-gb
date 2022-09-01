#pragma once

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "CPU/timer.h"
#include "utils/macro.h"

ALWAYS_INLINE void stack_push(u8 data)
{
    timer_tick();
    write_memory(--cpu.registers.sp, data);
}

ALWAYS_INLINE void stack_push_16bit(u16 data)
{
    stack_push(MSB(data));
    stack_push(LSB(data));
}

ALWAYS_INLINE u8 stack_pop()
{
    timer_tick();
    return read_memory((cpu.registers.sp)++);
}

ALWAYS_INLINE u16 stack_pop_16bit()
{
    const u8 lsb = stack_pop();
    const u8 msb = stack_pop();
    return lsb | (msb << 8);
}
