#include "io.h"

#include "cpu/cpu.h"
#include "cpu/interrupt.h"
#include "cpu/timer.h"
#include "options.h"
#include "utils/log.h"
#include "utils/macro.h"

void write_io(u16 address, u8 data)
{
    if (BETWEEN(address, TIMER_DIV, TIMER_TAC)) {
        write_timer(address, data);
        return;
    }

    switch (address) {
    case IF_ADDRESS:
        write_interrupt(IF_ADDRESS, data);
        break;

    default:
        // log_err("Unsupported IO write: " HEX, address);
        cpu.memory[address] = data;
    }
}

u8 read_io(u16 address)
{
    if (BETWEEN(address, TIMER_DIV, TIMER_TAC)) {
        return read_timer(address);
    }

    switch (address) {
    case IF_ADDRESS:
        return read_interrupt(IF_ADDRESS);

    default:
        // log_err("Unsupported IO read: " HEX, address);
        return cpu.memory[address];
    }
}
