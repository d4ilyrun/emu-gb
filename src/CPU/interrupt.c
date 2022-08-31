#include "CPU/interrupt.h"

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "CPU/stack.h"
#include "utils/log.h"

// clang-format off

struct interrupt {
    u8 flag;
    interrupt_vector vector;
};

const char *interrupt_names[6] = {
    "IV_VBLANK",
    "IV_LCD",
    "IV_TIMA",
    "IV_SERIAL",
    "IV_JOYPAD",
    "IV_NONE",
};

#define NAME(int_) \
    interrupt_names[((int_) - 0x40) / 8]

const struct interrupt interrupt_table[] = {
    [IV_VBLANK] = {0x01, IV_VBLANK},
    [IV_LCD]    = {0x02, IV_LCD},
    [IV_TIMA]   = {0x04, IV_TIMA},
    [IV_SERIAL] = {0x08, IV_SERIAL},
    [IV_JOYPAD] = {0x10, IV_JOYPAD}
};

// clang-format on

#define CHECK_INTERRUPT(_i)                                                 \
    do {                                                                    \
        if (val_ie & val_if & interrupt_table[(_i)].flag) {                 \
            log_info("Handling interrupt: %s", NAME(_i));                   \
            /* jump to the corresponding vector */                          \
            stack_push_16bit(read_register_16bit(REG_PC));                  \
            write_register_16bit(REG_PC, interrupt_table[(_i)].vector);     \
                                                                            \
            /* clear interrupt flags and deactivate halt mode */            \
            write_memory(IF_ADDRESS, val_if & ~interrupt_table[(_i)].flag); \
            cpu.halt = false;                                               \
                                                                            \
            return 5;                                                       \
        }                                                                   \
    } while (0);

static bool ime = true;

// TODO: verify clock cycles
u8 handle_interrupts()
{
    if (!ime)
        return 0;

    u8 val_ie = read_memory(IE_ADDRESS);
    u8 val_if = read_memory(IF_ADDRESS);

    // Faster to unroll the for loop by hand than going through it
    CHECK_INTERRUPT(IV_VBLANK);
    CHECK_INTERRUPT(IV_LCD);
    CHECK_INTERRUPT(IV_TIMA);
    CHECK_INTERRUPT(IV_SERIAL);
    CHECK_INTERRUPT(IV_JOYPAD);

    return 0;
}

bool interrupt_get_ime()
{
    return ime;
}

void interrupt_set_ime(bool value)
{
    ime = value;
}

void interrupt_request(interrupt_vector interrupt)
{
    u8 val_if = read_memory(IF_ADDRESS);
    write_memory(IF_ADDRESS, val_if | interrupt_table[interrupt].flag);

    log_info("Requested interrupt: %s", NAME(interrupt));
}
