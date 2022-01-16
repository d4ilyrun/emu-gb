#include "CPU/timer.h"

#include <stdio.h>

#include "CPU/interrupt.h"
#include "CPU/memory.h"
#include "utils/macro.h"

void reset_timer()
{
    write_memory_16bit(TIMER_DIV - 1, TIMER_DIV_DEFAULT);
}

void write_timer(u16 address, u8 data)
{
    switch ((timer_registers)address) {
    // DIV can be written but its value resets to 0 no matter what the written
    // value is.
    case TIMER_DIV:
        write_memory_16bit(address, 0x0000);
        break;
    case TIMER_TAC:
        write_memory(address, data & 0x07); // Only the lower 3 bites are R/W
        break;
    case TIMER_TMA:
    case TIMER_TIMA:
        write_memory(address, data);
        break;
    // TODO: error printing macro
    case TIMER_UNKNOWN:
        fprintf(stderr,
                ">> Timer: trying to write into an unknown register "
                "(" HEX ")\n",
                address);
        break;
    }
}

u8 read_timer(u16 address)
{
    switch ((timer_registers)address) {
    case TIMER_TAC:
        return read_memory(address) & 0x07; // Only the lower 3 bites are R/W
    case TIMER_DIV:
    case TIMER_TMA:
    case TIMER_TIMA:
        return read_memory(address);
    // TODO: error printing macro
    case TIMER_UNKNOWN:
        fprintf(stderr,
                ">> Timer: trying to read into an unknown register "
                "(" HEX ")\n",
                address);
        return 0;
    }
}

// The frequency at which we update TMA depends on the 2 lower bits of TAC
static u16 freq_divider[] = {8, 16, 64, 256};

void timer_ticks(u8 nb_cycle)
{
    u16 div = read_memory_16bit(TIMER_DIV - 1);
    u8 tac  = read_timer(TIMER_TAC);

    // update DIV's 16bit value
    write_memory_16bit(TIMER_DIV - 1, div + nb_cycle);

    // We only update the timer's value at certain frequencies
    // The frequency depends on the 2 lower bits of TAC and is stored in
    // freq_divider
    u16 freq         = freq_divider[tac & 0x03];
    bool update_tima = false;
    while (!update_tima && nb_cycle > 0)
        update_tima = ((div + nb_cycle--) % freq) == 0;

    // If bit 2 of TAC is set to 0 then the timer is disabled
    if (update_tima && tac & 0x4) {
        u8 tima = read_timer(TIMER_TIMA);

        if (tima == 0xFF) { // overflow
            write_timer(TIMER_TIMA, read_timer(TIMER_TMA));
            interrupt_request(IV_TIMA);
        } else {
            write_timer(TIMER_TIMA, tima + nb_cycle / freq);
        }
    }
}