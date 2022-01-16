#pragma once

#include <stdio.h>

#define NOT_IMPLEMENTED(_feature) fprintf(stderr, ">> Not implemented: %s\n", _feature)

// nth bit from a number x
#define BIT(_x, _n) ((_x) & (0x1 << (_n)))

// reverse byte order
#define REVERSE(_x) (((_x) >> 4) & ((_x) << 4))

// most/least significant byte in a 16-bit long number
#define MSB(_x) (_x >> 8)
#define LSB(_x) (_x & 0x00FF)

// display code for 8/16-bit hexadecimal numbers
#define HEX8 "0x%02X"
#define HEX16 "0x%04X"
#define HEX HEX16

