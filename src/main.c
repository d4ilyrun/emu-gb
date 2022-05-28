#include <stdio.h>

#include "CPU/cpu.h"
#include "CPU/instruction.h"
#include "CPU/interrupt.h"
#include "CPU/timer.h"
#include "cartridge/cartridge.h"
#include "test_rom.h"
#include "utils/macro.h"

int main(int argc, char **argv)
{
    if (argc < 1) {
        fputs("No cartridge provided.", stderr);
        return 1;
    }

    load_cartridge(argv[1]);
    cartridge_info();

    reset_cpu();
    reset_timer();

    u8 cycles;

    while (cpu.is_running) {
        if (cpu.halt) {
            cycles = 1;
        } else {
            cycles = execute_instruction();
        }

        timer_ticks(cycles * CYCLE_TICKS);

        handle_interrupts();

        if (cpu.ime_scheduled) {
            interrupt_set_ime(true);
            cpu.ime_scheduled = false;
            timer_ticks(CYCLE_TICKS);
        }

        test_rom_update();
        test_rom_print();
    }

    return 0;
}
