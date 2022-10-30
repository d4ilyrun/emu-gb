#pragma once

#include <stdlib.h>

#include "log.h"

#define NOT_IMPLEMENTED(...) log_err("Not implemented: %s", __VA_ARGS__)

#define FATAL_ERROR(...)      \
    do {                      \
        log_err(__VA_ARGS__); \
        exit(1);              \
    } while (0)

#define ASSERT_NOT_REACHED()                                         \
    do {                                                             \
        FATAL_ERROR("Unreachable code reached: %s:%s", __FUNCTION__, \
                    __LINE__);                                       \
    } while (0)

#define ASSERT_MSG(cond_, ...)        \
    do {                              \
        if (!(cond_)) {               \
            FATAL_ERROR(__VA_ARGS__); \
        }                             \
    } while (0)
