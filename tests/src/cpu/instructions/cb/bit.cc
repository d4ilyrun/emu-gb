// REG_ERR is also defined inside gtest ...
#include <algorithm>
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <CPU/cpu.h>
#include <CPU/instruction.h>
#include <utils/macro.h>
#include <utils/types.h>
}

#include "../instruction.hxx"

namespace cpu_tests
{

class Bit : public InstructionTest,
            public ::testing::WithParamInterface<cpu_register_name>
{
  public:
    Bit() : InstructionTest(2) {}
};

class BitRelative : public InstructionTest,
                    public ::testing::WithParamInterface<u16>
{
  public:
    BitRelative() : InstructionTest(2) {}
};

using Swap = Bit;

TEST_P(Bit, IsSet)
{
    const auto reg = GetParam();
    u8 inst[2] = {0xCB, 0x47};
    if (reg != REG_A)
        inst[1] = 0x40 + (reg - REG_B);

    u8 opcode = inst[1];

    const auto pc = cpu.registers.pc;

    for (u8 bit = 0; bit < 8; ++bit) {
        cpu.registers.pc = pc;
        inst[1] = opcode | (bit << 3);
        Load((void *)inst);
        for (u16 val = 0; val <= 0xFF; ++val) {
            cpu.registers.pc = pc;
            write_register(reg, val);
            execute_instruction();
            ASSERT_TRUE(get_flag(FLAG_H));
            ASSERT_FALSE(get_flag(FLAG_N));
            ASSERT_FALSE(get_flag(FLAG_C));
            ASSERT_EQ(get_flag(FLAG_Z), !BIT(val, bit));
        }
    }
}

TEST_P(Bit, Set)
{
    const auto reg = GetParam();
    u8 inst[2] = {0xCB, 0xC7};
    if (reg != REG_A)
        inst[1] = 0xC0 + (reg - REG_B);

    u8 opcode = inst[1];

    const auto pc = cpu.registers.pc;

    for (u8 bit = 0; bit < 8; ++bit) {
        cpu.registers.pc = pc;
        inst[1] = opcode | (bit << 3);
        Load((void *)inst);
        write_register(reg, 0);
        execute_instruction();
        ASSERT_TRUE(BIT(read_register(reg), bit));
    }
}

TEST_P(BitRelative, SetRelative)
{
    u8 inst[2] = {0xCB, 0xC6};
    const auto address = GetParam();

    u8 opcode = inst[1];

    const auto pc = cpu.registers.pc;
    for (u8 bit = 0; bit < 8; ++bit) {
        cpu.registers.pc = pc;
        inst[1] = opcode | (bit << 3);
        Load((void *)inst);

        write_register_16bit(REG_HL, address);
        write_memory(address, 0);
        execute_instruction();

        ASSERT_TRUE(BIT(read_memory(address), bit));
    }
}

TEST_P(Bit, Reset)
{
    const auto reg = GetParam();
    u8 inst[2] = {0xCB, 0x87};
    if (reg != REG_A)
        inst[1] = 0x80 + (reg - REG_B);

    u8 opcode = inst[1];

    const auto pc = cpu.registers.pc;

    for (u8 bit = 0; bit < 8; ++bit) {
        cpu.registers.pc = pc;
        inst[1] = opcode | (bit << 3);
        Load((void *)inst);
        write_register(reg, 0xFF);
        execute_instruction();
        ASSERT_FALSE(BIT(read_register(reg), bit));
    }
}

TEST_P(Swap, SwapNibbles)
{
    const auto pc = cpu.registers.pc;
    const auto reg = GetParam();

    u8 inst[2] = {0xCB, 0x37};
    if (reg != REG_A)
        inst[1] = 0x30 + (reg - REG_B);
    Load((void *)inst);

    for (u16 val = 0; val <= 0xFF; ++val) {
        cpu.registers.pc = pc;
        write_register(reg, val);
        execute_instruction();

        const auto res = read_register(reg);
        ASSERT_EQ(res & 0xF, (val & 0xF0) >> 4);
        ASSERT_EQ(val & 0xF, (res & 0xF0) >> 4);

        ASSERT_FALSE(get_flag(FLAG_N));
        ASSERT_FALSE(get_flag(FLAG_C));
        ASSERT_FALSE(get_flag(FLAG_H));
        ASSERT_EQ(get_flag(FLAG_Z), val == 0);
    }
}

CASES(registers, cpu_register_name) = {REG_A, REG_B, REG_C, REG_D,
                                       REG_E, REG_H, REG_L};

INSTANTIATE_TEST_SUITE_P(Registers, Bit,
                         ::testing::ValuesIn(registers_cpu_register_name));
INSTANTIATE_TEST_SUITE_P(Registers, Swap,
                         ::testing::ValuesIn(registers_cpu_register_name));

INSTANTIATE_TEST_SUITE_P(Relative, BitRelative, ::testing::Values(0x1FFF));

} // namespace cpu_tests
