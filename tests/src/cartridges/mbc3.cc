#define REG_ERR REG_ERR_
#include <gtest/gtest-death-test.h>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <CPU/cpu.h>
#include <CPU/memory.h>
#include <cartridge/cartridge.h>
#include <cartridge/memory.h>
}

#include "cartridge.hxx"

namespace cartridge_tests
{

struct mbc3_rtc_register {
    /// Actual computer clock, cannot be accessed directly.
    /// Its content shall be latched into the following registers to be read.
    time_t time;

    /// These registers are accessed when performing a read/write access
    /// They contain: seconds, minutes, hours, days (u16)
    u8 writable[5];
    u8 readable[5];
} rtc;

extern struct mbc3_rtc_register rtc;
extern u8 rtc_mapped_register;

// TODO

}; // namespace cartridge_tests
