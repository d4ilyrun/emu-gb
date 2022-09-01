#include "CPU/interrupt.h"

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "CPU/stack.h"
#include "utils/log.h"

#define FLAG(int_) (1 << ((int_)-0x40) / 8)
#define NAME(int_) interrupt_names[((int_)-0x40) / 8]

const char *interrupt_names[6] = {
    "IV_VBLANK", "IV_LCD", "IV_TIMA", "IV_SERIAL", "IV_JOYPAD", "IV_NONE",
};

struct interrupt_regsiter {
    bool vblank;
    bool lcd;
    bool tima;
    bool serial;
    bool joypad;
};

static u8 if_reg;
static u8 ie_reg;
static bool ime = true;

u8 read_interrupt(u16 address)
{
    if (address == IF_ADDRESS)
        return if_reg;
    else if (address == IE_ADDRESS)
        return ie_reg;

    log_err("Reading invalid interrupt register: " HEX ". Skipping", address);

    return 0;
}

void write_interrupt(u16 address, u8 val)
{
    if (address == IF_ADDRESS)
        if_reg = val;
    else if (address == IE_ADDRESS)
        ie_reg = val;
    else
        log_err("Writing invalid interrupt register: " HEX ". Skipping",
                address);
}

void interrupt_request(interrupt_vector interrupt)
{
    if_reg = if_reg | FLAG(interrupt);
    // log_trace("Requested [%s, %02X]", NAME(interrupt), FLAG(interrupt));
}

bool interrupt_get_ime()
{
    return ime;
}

void interrupt_set_ime(bool value)
{
    log_trace("%s IME", value ? "Set" : "Unset");
    ime = value;
}

bool interrupt_is_set(interrupt_vector interrupt)
{
    return if_reg & FLAG(interrupt);
}

static inline bool interrupt_is_enabled(interrupt_vector interrupt)
{
    return ie_reg & FLAG(interrupt);
}

// TODO: verify clock cycles
u8 handle_interrupts()
{
    for (interrupt_vector i = 0; i <= IV_JOYPAD; ++i) {
        if (interrupt_is_set(i) & interrupt_is_enabled(i)) {
            /* Exit halt mode */
            cpu.halt = false;

            /* Interrupts disabled */
            if (!ime)
                return 0;

            /* log_trace("Handling interrupt: %s", NAME(_i)); */
            /* jump to the corresponding vector */
            stack_push_16bit(read_register_16bit(REG_PC));
            write_register_16bit(REG_PC, i);

            /* clear interrupt flags and deactivate halt mode */
            write_memory(IF_ADDRESS, read_interrupt(IF_ADDRESS) & ~FLAG(i));
            cpu.halt = false;

            return 5;
        }
    }

    return 0;
}
