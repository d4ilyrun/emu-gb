// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <cpu/cpu.h>
#include <cpu/instruction.h>
#include <utils/log.h>
}

#include "../instruction.hxx"

namespace cpu_tests
{

class RLA : public InstructionTest
{
  public:
    RLA() : InstructionTest(1) {}
};

using RLCA = RLA;

#define TEST_RLA(c_)                                         \
    cpu.registers.pc = pc;                                   \
    set_flag(FLAG_C, (c_));                                  \
    write_register(reg, val);                                \
    timer.div = 0;                                           \
    execute_instruction();                                   \
    ASSERT_EQ(get_flag(FLAG_C), BIT(val, 7) ? 1 : 0);        \
    ASSERT_EQ(read_register(reg) & 0xFE, (val << 1) & 0xFE); \
    ASSERT_EQ(get_flag(FLAG_Z), false);

TEST_F(RLA, ImmediateValue)
{
    const auto &reg = REG_A;
    u8 instr[1] = {0x17};
    const auto pc = cpu.registers.pc;

    InstructionTest::Load((void *)instr);

    for (u16 val = 0; val <= 0xFF; ++val) {
        TEST_RLA(true);
        ASSERT_TRUE(read_register(reg) & 0x01);
        TEST_RLA(false);
        ASSERT_FALSE(read_register(reg) & 0x01);
    }
}

TEST_F(RLCA, ImmediateValue)
{
    const auto &reg = REG_A;
    u8 instr[1] = {0x07};
    const auto pc = cpu.registers.pc;

    InstructionTest::Load((void *)instr);

    for (u16 val = 0; val <= 0xFF; ++val) {
        TEST_RLA(true);
        ASSERT_EQ(BIT(cpu.registers.a, 0), BIT(val, 7));
        TEST_RLA(false);
        ASSERT_EQ(BIT(cpu.registers.a, 0), BIT(val, 7));
    }
}

} // namespace cpu_tests
