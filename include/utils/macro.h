#pragma once

#include <stdio.h>

// Convert preprocessors to string
#define xSTR(_x) #_x // NOLINT
#define STR(_x) xSTR(_x)

// Detect platform endianness
#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

#if !defined(__BYTE_ORDER__)
#error Undefined byte order. Please define __BYTE_ORDER__.
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define BIG_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN
#else
#error Unsupported endianness: xSTR(__BYTE_ORDER__)
#endif

#endif // LITTLE_ENDIAN && BIG_ENDIAN

// nth bit from a number x
#define BIT(_x, _n) (((_x) >> (_n)) & 0x1)

// reverse nybble/byte order
#define REVERSE(_x) ((((_x) >> 4) | ((_x) << 4)) & 0xFF)
#define REVERSE16(_x) ((((_x) >> 8) | ((_x) << 8)) & 0xFFFF)

// most/least significant byte in a 16-bit long number
#define MSB(_x) ((_x) >> 8)
#define LSB(_x) ((_x)&0x00FF)

// value is between a and b
#define BETWEEN(x_, a_, b_) ((x_) >= (a_) && (x_) <= (b_))

// value is in interval [a;b[
#define IN_RANGE(x_, a_, b_) ((x_) >= (a_) && (x_) < ((b_)))

// display code for 8/16-bit hexadecimal numbers
#define HEX8 "0x%02X"
#define HEX16 "0x%04X"
#define HEX HEX16

// force GCC to compile inline functions even without optmization
#define ALWAYS_INLINE inline __attribute__((always_inline))
