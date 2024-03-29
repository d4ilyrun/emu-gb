#include <stdio.h>

#include "cartridge/cartridge.h"
#include "cpu/cpu.h"
#include "cpu/instruction.h"
#include "cpu/interrupt.h"
#include "cpu/timer.h"
#include "options.h"
#include "test_rom.h"
#include "utils/log.h"
#include "utils/macro.h"

int main(int argc, char **argv)
{
    const struct options *options_ptr = parse_options(argc, argv);

    load_cartridge(options_ptr->args[0]);
    cartridge_info();

    reset_cpu();
    reset_timer();

    while (g_cpu.is_running) {
        if (g_cpu.halt) {
            timer_tick();
        } else {
            execute_instruction();
        }

        handle_interrupts();

        if (options_ptr->blargg) {
            test_rom_update();
            test_rom_print();
        }
    }

    return 0;
}
