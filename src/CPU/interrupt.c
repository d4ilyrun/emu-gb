#include "CPU/interrupt.h"

#include "CPU/cpu.h"
#include "CPU/memory.h"
#include "CPU/stack.h"
#include "CPU/timer.h"
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
    log_trace("Requested [%s, %02X]", NAME(interrupt), FLAG(interrupt));
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

static inline void handle_interrupt(interrupt_vector interrupt)
{
    log_trace("Handling interrupt: %s", NAME(interrupt));

    timer_ticks(2);
    stack_push_16bit(read_register_16bit(REG_PC)); // 2 timer ticks
    timer_tick();
    write_register_16bit(REG_PC, interrupt);
}

// TODO: verify clock cycles
u8 handle_interrupts()
{
    for (interrupt_vector i = 0; i <= IV_JOYPAD; ++i) {
        if (interrupt_is_set(i) & interrupt_is_enabled(i)) {
            /* Exit halt mode regardless of the value inside IME */
            cpu.halt = false;

            /* Interrupts disabled */
            if (!ime)
                return 0;

            handle_interrupt(i);

            /* clear interrupt flag */
            write_interrupt(IF_ADDRESS, read_interrupt(IF_ADDRESS) & ~FLAG(i));

            return 5;
        }
    }

    return 0;
}
