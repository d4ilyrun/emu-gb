#pragma once

#include "utils/types.h"

#define IF_ADDRESS 0xFF0F
#define IE_ADDRESS 0xFFFF

// Interrupt vectors
typedef enum interrupt_vector
{
    IV_NONE = 0x0000,
    IV_VBLANK = 0x0040,
    IV_LCD = 0x0048,
    IV_TIMA = 0x0050,
    IV_SERIAL = 0x0058,
    IV_JOYPAD = 0x0060,
} interrupt_vector;

struct interrupt {
    u8 flag;
    interrupt_vector vector;
};

u8 handle_interrupts();
void interrupt_set_ime(bool value);
void interrupt_request(interrupt_vector interrupt);
