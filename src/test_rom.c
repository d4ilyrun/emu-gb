#include "test_rom.h"

#include "cpu/memory.h"
#include "utils/log.h"

static char g_output[1024] = {0};
static u16 g_output_size = 0;

#define TEST_CHECK 0xFF02
#define TEST_VALUE 0xFF01

#define CHECK_TRUE 0x81

void test_rom_update()
{
    if (read_memory(TEST_CHECK) == CHECK_TRUE) {
        g_output[g_output_size++] = (char)read_memory(TEST_VALUE);
        write_memory(TEST_CHECK, 0);
    }
}

void test_rom_print()
{
    static u16 last_output_size = 0;

    if (g_output[0] && g_output_size > last_output_size) {
        log_info("test result: %s", g_output);
        last_output_size = g_output_size;
    }
}
