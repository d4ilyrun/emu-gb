#include "test_rom.h"

#include "CPU/memory.h"
#include "utils/log.h"

static char output[1024] = {0};
static u16 output_size = 0;

#define TEST_CHECK 0xFF02
#define TEST_VALUE 0xFF01

#define CHECK_TRUE 0x81

void test_rom_update()
{
    if (read_memory(TEST_CHECK) == CHECK_TRUE) {
        output[output_size++] = (char)read_memory(TEST_VALUE);
        write_memory(TEST_CHECK, 0);
    }
}

void test_rom_print()
{
    static u16 last_output_size = 0;

    if (output[0] && output_size > last_output_size) {
        log_info("test result: %s", output);
        last_output_size = output_size;
    }
}
