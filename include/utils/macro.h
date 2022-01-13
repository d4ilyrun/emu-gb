#pragma once

#include <stdio.h>

#define NOT_IMPLEMENTED(_feature) fprintf(stderr, ">> Not implemented: %s\n", _feature)

// nth bit from a number x
#define BIT(_x, _n) ((_x) & (0x1 << (_n)))

// reverse byte order
#define REVERSE(_x) (((_x) >> 4) & ((_x) << 4))