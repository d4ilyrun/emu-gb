#include "test_rom.h"

#include <stdio.h>

#include "CPU/memory.h"
#include "utils/macro.h"

static char output[1024] = {0};
static u16 ouput_size = 0;

void test_rom_update()
{
    if (read_memory(TEST_CHECK) == CHECK_TRUE) {
        output[ouput_size++] = (char)read_memory(TEST_VALUE);
        write_memory(TEST_CHECK, 0);
    } else if (read_memory(TEST_CHECK)) {
        printf(HEX "=" HEX "\n", TEST_CHECK, read_memory(TEST_CHECK));
    }
}

void test_rom_print()
{
    if (output[0])
        printf(">> test result: %s\n", output);
}
