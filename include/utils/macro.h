#pragma once

#include <stdio.h>

// nth bit from a number x
#define BIT(_x, _n) (((_x) >> (_n)) & 0x1)

// reverse nybble order
#define REVERSE(_x) (((_x) >> 4) & ((_x) << 4))

// most/least significant byte in a 16-bit long number
#define MSB(_x) (_x >> 8)
#define LSB(_x) (_x & 0x00FF)

// display code for 8/16-bit hexadecimal numbers
#define HEX8 "0x%02X"
#define HEX16 "0x%04X"
#define HEX HEX16

// force GCC to compile inline functions even without optmization
#define ALWAYS_INLINE inline __attribute__((always_inline))
