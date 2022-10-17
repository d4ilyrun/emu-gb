#include <stdio.h>
#include <stdlib.h>

#include "cartridge/cartridge.h"
#include "cpu/cpu.h"
#include "cpu/instruction.h"
#include "cpu/interrupt.h"
#include "cpu/timer.h"
#include "options.h"
#include "ppu/ppu.h"
#include "test_rom.h"
#include "utils/error.h"
#include "utils/log.h"
#include "utils/macro.h"

#include "pthread.h"
#include "gui/sdl.h"

int main(int argc, char **argv)
{
    const struct options *options = parse_options(argc, argv);
    pthread_t gui_thread;

    load_cartridge(options->args[0]);
    cartridge_info();

    if (options->gui) {
        gui_init(options->args[0]);
        pthread_create(&gui_thread, NULL, gui_main, NULL);
    }

    reset_cpu();
    ppu_init();
    reset_timer();

    while (cpu.is_running) {
        if (cpu.halt) {
            timer_tick();
        } else {
            execute_instruction();
        }

        handle_interrupts();

        if (options->blargg) {
            test_rom_update();
            test_rom_print();
        }
    }
}
