#pragma once

#include "utils/types.h"

// Initial value for tje dov register (when PC=0x0100).
// This value depends on the model, but for the original Game Boy (DMG) it is 0xABCC.
#define TIMER_DIV_DEFAULT   0xABCC

// Addresses of the different timer registers
typedef enum timer_registers
{
    TIMER_UNKNOWN = 0x0000,
    TIMER_DIV     = 0xFF04,
    TIMER_TIMA    = 0xFF05,
    TIMER_TMA     = 0xFF06,
    TIMER_TAC     = 0xFF07,
} timer_registers;

void reset_timer();

void write_timer(u16 address, u8 data);
u8 read_timer(u16 address);

void timer_ticks(u8 ticks);