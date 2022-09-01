// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <CPU/cpu.h>
#include <CPU/instruction.h>
#include <utils/log.h>
}

#include "../instruction.hxx"

namespace cpu_tests
{

template <uint size, typename param_type>
class SRA : public InstructionTest,
            public ::testing::WithParamInterface<param_type>
{
  public:
    SRA() : InstructionTest(size) {}
};

using SRA_Register = SRA<2, cpu_register_name>;
class SRA_Relative : public InstructionTest
{
  public:
    SRA_Relative() : InstructionTest(2) {}
};

TEST_F(SRA_Relative, RightRotate)
{
    const auto pc = cpu.registers.pc;
    u8 instr[2] = {0xCB, 0x2E};
    InstructionTest::Load((void *)instr);

    const u16 address = 0x1234;
    cpu.registers.h = MSB(address);
    cpu.registers.l = LSB(address);

    for (u16 val = 0; val <= 0xFF; ++val) {
        cpu.registers.pc = pc;
        write_memory(address, val);
        timer.div = 0;
        execute_instruction();
        ASSERT_EQ(get_flag(FLAG_C), val & 0x1);                    // Flag C
        ASSERT_EQ(read_memory(address) & 0x7F, (val >> 1) & 0x7F); // Core
        ASSERT_EQ(read_memory(address) & 0x80, val & 0x80);        // Bit 7
    }
}

TEST_P(SRA_Register, RightRotate)
{
    const auto &reg = GetParam();
    u8 instr[2] = {0xCB, 0x2F};
    const auto pc = cpu.registers.pc;

    ASSERT_TRUE(!IS_16BIT(reg));

    if (reg != REG_A)
        instr[1] = 0x28 + (reg - REG_B);

    InstructionTest::Load((void *)instr);

    for (u16 val = 0; val <= 0xFF; ++val) {
        cpu.registers.pc = pc;
        write_register(reg, val);
        timer.div = 0;
        execute_instruction();
        ASSERT_EQ(get_flag(FLAG_C), val & 0x1);                  // Flag C
        ASSERT_EQ(read_register(reg) & 0x7F, (val >> 1) & 0x7F); // Core
        ASSERT_EQ(read_register(reg) & 0x80, val & 0x80);        // Bit 7
    }
}

INSTANTIATE_TEST_SUITE_P(Registers, SRA_Register,
                         ::testing::Values(REG_B, REG_C, REG_D, REG_E, REG_H,
                                           REG_L, REG_A));

} // namespace cpu_tests
