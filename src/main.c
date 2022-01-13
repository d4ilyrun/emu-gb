#include <stdio.h>

#include "cartridge.h"

int main(int argc, char **argv)
{
    if (argc < 1)
    {
        fputs("No cartridge provided.", stderr);
        return 1;
    }

    load_cartridge(argv[1]);
    cartridge_info();

    return 0;
}