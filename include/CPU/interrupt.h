#pragma once

#include "utils/types.h"

#define IE_ADDRESS 0xFFFF
#define IF_ADDRESS 0xFF0F

// TODO: Comments ! What does all this refer to !?

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

u8 handle_interrupts();
void interrupt_set_ime(bool value);
bool interrupt_get_ime();
void interrupt_request(interrupt_vector interrupt);
