#include "CPU/interrupt.h"

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "CPU/stack.h"

struct interrupt interrupt_table[] = {[IV_VBLANK] = {0x01, IV_VBLANK},
                                      [IV_LCD]    = {0x02, IV_LCD},
                                      [IV_TIMA]   = {0x04, IV_TIMA},
                                      [IV_SERIAL] = {0x08, IV_SERIAL},
                                      [IV_JOYPAD] = {0x10, IV_JOYPAD}};

#define CHECK_INTERRUPT(_i)                                                 \
    do {                                                                    \
        if (val_ie & val_if & interrupt_table[(_i)].flag) {                 \
            /* jump to the corresponding vector */                          \
            stack_push_16bit(read_register_16bit(REG_PC));                     \
            write_register_16bit(REG_PC, interrupt_table[(_i)].vector);        \
                                                                            \
            /* clear interrupt flags */                                     \
            write_memory(IF_ADDRESS, val_if & ~interrupt_table[(_i)].flag); \
                                                                            \
            return 5;                                                       \
        }                                                                   \
    } while (0);

static bool ime = true;

// TODO: HALT mode interrupt
// TODO: verify clock cycles
u8 handle_interrupts()
{
    if (!ime)
        return 0;

    u8 val_ie = read_memory(IE_ADDRESS);
    u8 val_if = read_memory(IF_ADDRESS);

    // Faster to unroll the for loop by hand than going through it
    CHECK_INTERRUPT(0);
    CHECK_INTERRUPT(1);
    CHECK_INTERRUPT(2);
    CHECK_INTERRUPT(3);
    CHECK_INTERRUPT(4);

    return 0;
}

void interrupt_set_ime(bool value)
{
    ime = value;
}

void interrupt_request(interrupt_vector interrupt)
{
    u8 val_if = read_memory(IF_ADDRESS);
    write_memory(IF_ADDRESS, val_if | interrupt_table[interrupt].flag);
}
