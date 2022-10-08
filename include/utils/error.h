#pragma once

#include <stdlib.h>

#include "log.h"

#define not_implemented(...) log_err("Not implemented: %s", __VA_ARGS__)

#define fatal_error(...)      \
    do {                      \
        log_err(__VA_ARGS__); \
        exit(1);              \
    } while (0);

#define assert_not_reached()                                                    \
    do {                                                                        \
        fatal_error("Unreachable code reached: %s:%s", __FUNCTION__, __LINE__); \
    } while (0);

#define assert_msg(cond_, ...)        \
    do {                              \
        if (!(cond_)) {               \
            fatal_error(__VA_ARGS__); \
        }                             \
    } while (0);
