// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <cpu/cpu.h>
#include <cpu/instruction.h>
#include <utils/macro.h>
}

#include "ld.hxx"

namespace cpu_tests
{

struct ld_imm_params {
    u8 instruction[2];
    u8 *out; // cpu register
    u8 expected;
};

struct ld_imm_params_16 {
    u8 instruction[3];
    cpu_register_name out;
    u16 expected;
};

using LoadImmediate = LoadInstructionTest<2, ld_imm_params>;

class LoadSPAddition : public InstructionTest
{
  public:
    LoadSPAddition() : InstructionTest(2) {}

    void SetUp() override
    {
        InstructionTest::SetUp();
    }
};

class LoadImmediate16 : public InstructionTest,
                        public ::testing::WithParamInterface<ld_imm_params_16>
{
  public:
    LoadImmediate16() : InstructionTest(3) {}

    virtual void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
    }

    void TearDown() override
    {
        InstructionTest::TearDown();
        const auto &param = this->GetParam();
        ASSERT_EQ(param.expected, read_register_16bit(param.out));
    }
};

TEST_P(LoadImmediate, ImmediateValueToRegister)
{
    execute_instruction();
}

TEST_P(LoadImmediate16, ImmediateValueToRegister)
{
    execute_instruction();
}

CASES(registers, ld_imm_params) = {
    {{0b00111110, 0xFF}, &cpu.registers.a, 0xFF}, // A
    {{0b00000110, 0x42}, &cpu.registers.b, 0x42}, // B
    {{0b00001110, 0x00}, &cpu.registers.c, 0x00}, // C
    {{0b00010110, 0x0F}, &cpu.registers.d, 0x0F}, // D
    {{0b00011110, 0xF0}, &cpu.registers.e, 0xF0}, // E
    {{0b00100110, 0x08}, &cpu.registers.h, 0x08}, // H
    {{0b00101110, 0xA5}, &cpu.registers.l, 0xA5}, // L
};

INSTANTIATE_TEST_SUITE_P(Registers, LoadImmediate,
                         testing::ValuesIn(registers_ld_imm_params));

CASES(registers, ld_imm_params_16) = {
    {{0x01, 0xFF, 0xFF}, REG_BC, 0xFFFF}, // BC
    {{0x11, 0x24, 0x42}, REG_DE, 0x4224}, // DE
    {{0x21, 0xF0, 0x0F}, REG_HL, 0x0FF0}, // HL
    {{0x31, 0x80, 0x08}, REG_SP, 0x0880}, // SP
};

INSTANTIATE_TEST_SUITE_P(Registers, LoadImmediate16,
                         ::testing::ValuesIn(registers_ld_imm_params_16));

}; // namespace cpu_tests
