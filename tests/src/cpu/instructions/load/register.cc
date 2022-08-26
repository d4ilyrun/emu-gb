// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <CPU/cpu.h>
#include <CPU/instruction.h>
#include <utils/macro.h>
}

#include "ld.hxx"

namespace cpu_tests
{

struct load_params_reg {
    u8 instruction[1] = {0};
    u8 *out; // cpu register pointer
    u8 *in;  // cpu register pointer
    u8 expected = 0x1F;
};

class LoadFromRegister : public LoadInstructionTest<0x1, struct load_params_reg>
{
  public:
    LoadFromRegister() : LoadInstructionTest() {}

    static std::vector<load_params_reg> cases;
};

// Testing each register at least once should suffice
std::vector<load_params_reg> LoadFromRegister::cases{
    {0x47, &cpu.registers.b, &cpu.registers.a},
    {0x40, &cpu.registers.b, &cpu.registers.b},
    {0x41, &cpu.registers.b, &cpu.registers.c},
    {0x42, &cpu.registers.b, &cpu.registers.d},
    {0x58, &cpu.registers.e, &cpu.registers.b},
    {0x60, &cpu.registers.h, &cpu.registers.b},
    {0x68, &cpu.registers.l, &cpu.registers.b},
};

TEST_P(LoadFromRegister, RegisterToRegister)
{
    const auto &param = GetParam();
    *param.in = param.expected;
    execute_instruction();
};

INSTANTIATE_TEST_SUITE_P(LD_8bit, LoadFromRegister,
                         ::testing::ValuesIn(LoadFromRegister::cases));

class LoadFrom16BitRegister : public InstructionTest
{
  public:
    LoadFrom16BitRegister() : InstructionTest(1) {}
};

TEST_F(LoadFrom16BitRegister, HLToSP)
{
    const u8 instruction[1] = {0xF9};

    Load((void *)instruction);
    cpu.registers.h = 0x12;
    cpu.registers.l = 0x34;
    execute_instruction();

    ASSERT_EQ(cpu.registers.sp, 0x1234);
}

} // namespace cpu_tests
