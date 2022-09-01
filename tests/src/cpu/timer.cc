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
        ASSERT_EQ(read_timer(TIMER_TIMA), i / 16);
    }
}

TEST_F(TimerTIMA, MultipleTicks)
{
    // Enabled + freq = 16 clock cycles
    const u8 tac = 0b101;

    write_memory(TIMER_TAC, tac);
    write_memory(TIMER_TIMA, 0);

    timer_ticks(16 * 4);
    ASSERT_EQ(read_timer(TIMER_TIMA), 4);
}

TEST_F(TimerTIMA, Overflow)
{
    // Enabled + freq = 16 clock cycle
    const u8 tac = 0b101;
    const u8 tma = 0x15;

    write_memory(TIMER_TMA, tma);
    write_memory(TIMER_TAC, tac);
    write_memory(TIMER_TIMA, 0xFF);

    timer_ticks(16);

    const u8 if_reg = read_memory(IF_ADDRESS);
    ASSERT_EQ(read_timer(TIMER_TIMA), 0);
    ASSERT_FALSE(BIT(if_reg, 2)); // Delayed

    timer_tick();
    ASSERT_EQ(read_timer(TIMER_TIMA), tma);
    ASSERT_TRUE(BIT(if_reg, 2)); // should be set now
}

} // namespace cpu_tests
