#include "cpu/timer.h"

#include <stdio.h>

#include "cpu/cpu.h"
#include "cpu/interrupt.h"
#include "cpu/memory.h"
#include "utils/error.h"
#include "utils/log.h"
#include "utils/macro.h"

#define CLOCKS_PER_CYCLE 4

static bool tima_overflow = false;

struct timer {
    u16 div;
    u8 tima;
    u8 tma;
    u8 tac;
} timer;

void reset_timer()
{
    timer.div = TIMER_DIV_DEFAULT;
}

void write_timer(u16 address, u8 data)
{
    switch ((timer_registers)address) {
    // DIV can be written but its value resets to 0 no matter what the written
    // value is.
    case TIMER_DIV:
        timer.div = 0x0000; // Reset the whole DIV
        break;
    case TIMER_TAC:
        // Only the lower 3 bites are R/W
        timer.tac = (timer.tac & ~0b111) | (data & 0b111);
        break;
    case TIMER_TMA:
        timer.tma = data;
        break;
    case TIMER_TIMA:
        timer.tima = data;
        break;

    default:
    case TIMER_UNKNOWN:
        log_warn("Invalid timer write: (" HEX16 "). Skipping", address);
        break;
    }
}

u8 read_timer(u16 address)
{
    switch ((timer_registers)address) {
    case TIMER_TAC:
        return timer.tac & 0b111; // Only the lower 3 bites are R/W
    case TIMER_DIV:
        return MSB(timer.div);
    case TIMER_TMA:
        return timer.tma;
    case TIMER_TIMA:
        return timer.tima;

    default:
    case TIMER_UNKNOWN:
        log_warn("Invalid timer read: (" HEX16 "). Skipping", address);
        return 0;
    }
}

// Number of clocks at which we update TIMA
// The frequency at which we update TIMA depends on the 2 lower bits of TAC
static u16 freq_divider[] = {1024, 16, 64, 256};

void timer_ticks(u8 ticks)
{
    u16 div = timer.div;
    u8 tac = read_timer(TIMER_TAC);

    // update DIV's 16bit value
    timer.div += ticks;

    // delayed IE
    if (cpu.ime_scheduled) {
        interrupt_set_ime(true);
        cpu.ime_scheduled = false;
    }

    // TIMA overflowed during the last cycle
    if (tima_overflow) {
        tima_overflow = false;
        interrupt_request(IV_TIMA);
        timer.tima = timer.tma;
    }

    // We only update the timer's value at certain frequencies (freq_divider)
    // Here we compute the number of 'freq' between the old div and the new div
    // (in clocks, no cycles ! Hence we divide by 4)
    u16 freq = freq_divider[tac & 0x03] / 4;
    u8 increase_tima = ((div + ticks) / freq) - (div / freq);

    // If bit 2 of TAC is set to 0 then the timer is disabled
    if (increase_tima && tac & 0x4) {
        u8 tima = read_timer(TIMER_TIMA);

        if (tima == 0xFF) { // overflow
            // Timer interrupt is delayed 1 cycle (4 clocks) from the TIMA
            // overflow. The TMA reload to TIMA is also delayed. For one cycle,
            // after overflowing TIMA, the value in TIMA is 00h, not TMA.
            write_timer(TIMER_TIMA, 0x00);
            tima_overflow = true;
        } else {
            write_timer(TIMER_TIMA, tima + increase_tima);
        }
    }
}
