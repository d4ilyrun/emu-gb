// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

#include "../cartridges/cartridge.hxx"

extern "C" {
#include <CPU/cpu.h>
#include <CPU/instruction.h>
#include <CPU/interrupt.h>
#include <CPU/memory.h>
#include <CPU/timer.h>
#include <options.h>
#include <utils/error.h>
#include <utils/macro.h>
}

#define call(opcode_)                          \
    write_memory(cpu.registers.pc, (opcode_)); \
    execute_instruction();

namespace cpu_tests
{

class TimerTest : public ::testing::Test
{
  public:
    TimerTest()
    {
        const auto cart = CartridgeGenerator<1 << 18>(ROM_ONLY);
        cartridge = cart.GetCart();
    }

    void SetUp() override
    {
        reset_cpu();
        reset_timer();
        interrupt_set_ime(true);
    }
};

using TimerDIV = TimerTest;
using TimerTIMA = TimerTest;
using TimerTAC = TimerTest;

TEST_F(TimerDIV, DefaultValue)
{
    ASSERT_EQ(read_timer(TIMER_DIV), 0xAB);
}

TEST_F(TimerDIV, Tick)
{
    write_memory(TIMER_TAC, 0b101);
    write_memory(TIMER_DIV, 0); // Reset regiser to 0

    for (u32 tick = 1; tick <= 0xFFFF; ++tick) {
        timer_tick();
        ASSERT_EQ(read_timer(TIMER_DIV), tick >> 8);
    }
}

TEST_F(TimerDIV, ResetOnWrite)
{
    // Not 0 by default
    write_memory(TIMER_DIV, 0x1F);
    ASSERT_EQ(read_memory(TIMER_DIV), 0);
}

TEST_F(TimerDIV, ResetOnStop)
{
    call(0x10); // STOP
    ASSERT_EQ(read_memory(TIMER_DIV), 0);
}

TEST_F(TimerTIMA, Tick)
{
    // Enabled + freq = 16 clock cycles
    const u8 tac = 0b101;

    write_memory(TIMER_TAC, tac);
    write_memory(TIMER_TIMA, 0);
    write_memory(TIMER_DIV, 0);

    for (u16 i = 1; i <= 0xFF; ++i) {
        timer_tick();
        ASSERT_EQ(read_timer(TIMER_TIMA), i / (16 / 4));
    }
}

TEST_F(TimerTIMA, MultipleTicks)
{
    // Enabled + freq = 16 clock cycles
    const u8 tac = 0b101;

    write_memory(TIMER_TAC, tac);
    write_memory(TIMER_TIMA, 0);

    timer_ticks(16);
    ASSERT_EQ(read_timer(TIMER_TIMA), 4);
}

class OverflowTIMA : public TimerTIMA, public ::testing::WithParamInterface<u8>
{
};

TEST_P(OverflowTIMA, Overflow)
{
    static u16 freq_divider[] = {1024, 16, 64, 256};

    const u8 tma = 0x15;
    const u8 tac = GetParam();

    write_memory(IE_ADDRESS, 0x04);
    write_memory(IF_ADDRESS, 0x00);
    write_memory(TIMER_TMA, tma);
    write_memory(TIMER_DIV, 0);

    write_memory(TIMER_TAC, tac);
    write_memory(TIMER_TIMA, 0xFF);

    const auto freq = freq_divider[tac & 0b11] / 4; // use clocks !
    for (auto i = 0; i < freq; ++i)
        timer_tick();

    ASSERT_EQ(read_timer(TIMER_TIMA), 0);
    ASSERT_FALSE(interrupt_is_set(IV_TIMA)); // Delayed

    timer_tick(); // Set value after overflow
    ASSERT_EQ(read_timer(TIMER_TIMA), tma);
    ASSERT_TRUE(interrupt_is_set(IV_TIMA));
}

INSTANTIATE_TEST_SUITE_P(Overflow, OverflowTIMA,
                         ::testing::Range<u8>(0b100, 0b1000, 1));

} // namespace cpu_tests
