#include <stdio.h>

#include "cartridge.h"
#include "CPU/cpu.h"
#include "CPU/instructions.h"

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

    while (1)
    {
        execute_instruction();
    }

    return 0;
}