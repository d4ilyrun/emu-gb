#pragma once

#include "utils/types.h"

// Initial value inside the DIV register.
// This value depends on the model. For the original Game Boy (DMG) it is
// 0xABCC.
#define TIMER_DIV_DEFAULT 0xABCC

// Addresses of the different timer registers
typedef enum timer_registers
{
    TIMER_UNKNOWN = 0x0000,
    TIMER_DIV = 0xFF04,
    TIMER_TIMA = 0xFF05,
    TIMER_TMA = 0xFF06,
    TIMER_TAC = 0xFF07,
} timer_registers;

/**
 * \function reset_timer
 * \brief Reset the internal timer to its default state
 */
void reset_timer();

/**
 * \function write_timer
 * \brief write an 8bit value into the internal timer's registers.
 *
 * \param address 16bit memory address between 0xFF04 - 0xFF07
 * \param val 8bit value
 */
void write_timer(u16 address, u8 data);

/**
 * \function read_timer
 * \brief read a 8bit value from the internal timer's registers.
 * \param address 16bit memory address between 0xFF04 - 0xFF07
 * \see read_memory
 */
u8 read_timer(u16 address);

/**
 * \function timer_ticks
 * \brief Add a certain amount of cycles to the CPU internal timer
 *
 * It will update the content inside the DIV register. Then update the other
 * registers if needed (at certain frequencies).
 */
void timer_ticks(u8 ticks);

// Add a single cycle to the timer
#define timer_tick() timer_ticks(1);
