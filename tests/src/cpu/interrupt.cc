// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

#include "../cartridges/cartridge.hxx"

extern "C" {
#include <cpu/cpu.h>
#include <cpu/instruction.h>
#include <cpu/interrupt.h>
#include <cpu/memory.h>
#include <cpu/stack.h>
#include <cpu/timer.h>
#include <options.h>
#include <utils/error.h>
#include <utils/macro.h>
}

struct timer {
    u16 div;
    u8 tima;
    u8 tma;
    u8 tac;
};

extern struct timer g_timer;

namespace cpu_tests
{

class InterrupTest : public ::testing::Test
{
  public:
    InterrupTest()
    {
        const auto cart = CartridgeGenerator<1 << 18>(ROM_ONLY);
        cartridge = cart.GetCart();
    }

    void SetUp() override
    {
        reset_cpu();
        interrupt_set_ime(true);
    }
};

using IME = InterrupTest;

#define call(opcode_)                            \
    write_memory(g_cpu.registers.pc, (opcode_)); \
    execute_instruction();

TEST_F(IME, Modify)
{
    get_options()->trace = true;

    // DI: Disable ime
    call(0xF3);
    ASSERT_FALSE(interrupt_get_ime());

    // EI: Enable ime (delayed by 1 instruction)
    call(0xFB);
    ASSERT_TRUE(g_cpu.ime_scheduled);
    ASSERT_FALSE(interrupt_get_ime());

    // Delay
    call(0x00);
    ASSERT_TRUE(interrupt_get_ime());

    // RETI: Enable ime + return
    // push pc
    const u16 pc = g_cpu.registers.pc;
    stack_push_16bit(g_cpu.registers.pc++);
    call(0xD9);
    ASSERT_EQ(g_cpu.registers.pc, pc);
    ASSERT_TRUE(interrupt_get_ime());
}

TEST_F(IME, HaltBug)
{
    get_options()->trace = true;

    interrupt_set_ime(false);

    // EI: Enable ime
    call(0xFB);
    ASSERT_TRUE(g_cpu.ime_scheduled);
    call(0xF3);
    ASSERT_FALSE(interrupt_get_ime());
}

class InterruptRequest : public InterrupTest,
                         public ::testing::WithParamInterface<interrupt_vector>
{
};

#define IF 0xFF0F
#define IE 0xFFFF

std::vector<interrupt_vector> interrupts = {
    IV_VBLANK, IV_JOYPAD, IV_SERIAL, IV_TIMA, IV_LCD,
};

static inline u8 interrupt_bit(interrupt_vector interrupt)
{
    return (interrupt - 0x40) / 8;
};

TEST_P(InterruptRequest, Request)
{
    const auto interrupt = GetParam();
    interrupt_request(interrupt);

    const u8 if_reg = read_memory(IF);
    const u8 bit = interrupt_bit(interrupt);

    ASSERT_TRUE(BIT(if_reg, bit));
}

TEST_P(InterruptRequest, Execute)
{
    const auto interrupt = GetParam();
    const u8 bit = interrupt_bit(interrupt);

    write_memory(IF, 1 << bit);
    write_memory(IE, 1 << bit);

    const auto cycles = handle_interrupts();

    ASSERT_EQ(cycles, 5);
    ASSERT_EQ(read_memory(IF), 0);
    ASSERT_EQ(read_memory(IE), 1 << bit); // Don't reset IE
    ASSERT_EQ(g_cpu.registers.pc, interrupt);
}

TEST_P(InterruptRequest, Timing)
{
    const auto interrupt = GetParam();
    const u8 bit = interrupt_bit(interrupt);

    g_timer.div = 0;
    write_memory(IF, 1 << bit);
    write_memory(IE, 1 << bit);

    const auto cycles = handle_interrupts();

    ASSERT_EQ(g_timer.div, 5);
}

TEST_P(InterruptRequest, ExecuteDisabledIME)
{
    const auto interrupt = GetParam();
    const u8 bit = interrupt_bit(interrupt);

    write_memory(IF, 1 << bit);
    write_memory(IE, 1 << bit);
    interrupt_set_ime(false); // Deactivate interrupts

    const auto cycles = handle_interrupts();

    ASSERT_EQ(cycles, 0);
}

TEST_P(InterruptRequest, Priority)
{
    const auto interrupt = GetParam();
    const u8 bit = interrupt_bit(interrupt);

    for (const auto &other : interrupts) {
        u8 flags = (1 << bit) | (1 << interrupt_bit(other));
        write_memory(IF, flags);
        write_memory(IE, flags);

        const auto cycles = handle_interrupts();

        ASSERT_EQ(cycles, 5);
        ASSERT_EQ(g_cpu.registers.pc, std::min(interrupt, other));

        if (interrupt != other) {
            const auto if_reg = read_memory(IF);
            ASSERT_EQ(if_reg, 1 << interrupt_bit(std::max(interrupt, other)));
        }
    }
}

INSTANTIATE_TEST_SUITE_P(Interrupts, InterruptRequest,
                         ::testing::ValuesIn(interrupts));

using InterrupRW = InterrupTest;

TEST_F(InterrupRW, IF)
{
    for (u16 i = 0; i <= 0xFF; ++i) {
        write_memory(IF, i);
        ASSERT_EQ(read_memory(IF), i);
    }
}

TEST_F(InterrupRW, IE)
{
    for (u16 i = 0; i <= 0xFF; ++i) {
        write_memory(IE, i);
        ASSERT_EQ(read_memory(IE), i);
    }
}

} // namespace cpu_tests
