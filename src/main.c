#include <stdio.h>

#include "CPU/cpu.h"
#include "CPU/instruction.h"
#include "CPU/interrupt.h"
#include "CPU/timer.h"
#include "cartridge/cartridge.h"
#include "options.h"
#include "test_rom.h"
#include "utils/log.h"
#include "utils/macro.h"

int main(int argc, char **argv)
{
    const struct options *options = parse_options(argc, argv);

    load_cartridge(options->args[0]);
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

        if (options->blargg) {
            test_rom_update();
            test_rom_print();
        }
    }

    return 0;
}
