#include <stdio.h>

#include "cartridge.h"
#include "CPU/cpu.h"
#include "CPU/timer.h"
#include "CPU/instruction.h"
#include "CPU/interrupt.h"
#include "utils/macro.h"

int main(int argc, char **argv)
{
    if (argc < 1)
    {
        fputs("No cartridge provided.", stderr);
        return 1;
    }

    load_cartridge(argv[1]);
    cartridge_info();

    reset_cpu();
    reset_timer();

    u8 cycles;

    while (cpu.is_running)
    {
        cycles = execute_instruction();

        timer_ticks(cycles * CYCLE_TICKS);

        handle_interrupts();

        if (cpu.ime_scheduled)
        {
            interrupt_set_ime(true);
            cpu.ime_scheduled = false;
            timer_ticks(CYCLE_TICKS);
        }
    }

    return 0;
}