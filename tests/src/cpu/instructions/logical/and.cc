// REG_ERR is also defined inside gtest ...
#include <algorithm>
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <cpu/cpu.h>
#include <cpu/flag.h>
#include <cpu/instruction.h>
#include <utils/macro.h>
#include <utils/types.h>
}

#include "../instruction.hxx"

namespace cpu_tests
{

template <uint size, typename param_type, cpu_register_name out = REG_A>
class AndTest : public InstructionTest,
                public ::testing::WithParamInterface<param_type>
{
  public:
    AndTest() : InstructionTest(size){};

    void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
    }

    void TearDown() override
    {
        const auto &param = this->GetParam();

        ASSERT_EQ(cpu.registers.a, param.x & param.y);

        ASSERT_EQ(get_flag(FLAG_C), param.flags.c);
        ASSERT_EQ(get_flag(FLAG_H), param.flags.h);
        ASSERT_EQ(get_flag(FLAG_N), param.flags.n);
        ASSERT_EQ(get_flag(FLAG_Z), param.flags.z);
    };
};

struct flags {
    u8 c = false;
    u8 h = true;
    u8 n = false;
    u8 z = false;
};

struct from_register {
    u8 x, y;
    struct flags flags;
    u8 instruction[1] = {0xA0};
    cpu_register_name reg = REG_B;
};

struct imm_d8 {
    u8 x;
    u8 instruction[1] = {0xE6};
    u8 y;
    struct flags flags;
};

struct hl_rel {
    u8 x, y;
    struct flags flags;
    u8 instruction[1] = {0xA6};
};

using AndFromRegister = AndTest<1, from_register>;
using AndImmediateValue = AndTest<2, imm_d8>;
using AndHLRelative = AndTest<1, hl_rel>;

TEST_P(AndFromRegister, Sub)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    write_register(param.reg, param.y);
    execute_instruction();
}

TEST_P(AndImmediateValue, Sub)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    execute_instruction();
}

TEST_P(AndHLRelative, Sub)
{
    const auto &param = GetParam();
    const u16 address = 0x7FFF;
    cpu.registers.a = param.x;
    write_register_16bit(REG_HL, address);
    write_memory(address, param.y);
    execute_instruction();
}

CASES(easy, from_register) = {
    {0x00, 0xFF, {.z = true}},
    {0xFF, 0x00, {.z = true}},
    {0xA9, 0xC1},
};

CASES(register, from_register) = {
    {0xA9, 0xC1, {}, {0xA1}, REG_C}, {0xA9, 0xC2, {}, {0xA2}, REG_D},
    {0xA9, 0xC3, {}, {0xA3}, REG_E}, {0xA9, 0xC4, {}, {0xA4}, REG_H},
    {0xA9, 0xC5, {}, {0xA5}, REG_L}, {0xC7, 0xC7, {}, {0xA7}, REG_A},
};

CASES(easy, imm_d8){
    {.x = 0x00, .y = 0xFF, .flags = {.z = true}},
    {.x = 0xFF, .y = 0x00, .flags = {.z = true}},
    {.x = 0xF2, .y = 0xC3},
};

CASES(easy, hl_rel) = {
    {0x00, 0xFF, {.z = true}},
    {0xFF, 0x00, {.z = true}},
    {0xA9, 0xC1},
};

using ::testing::ValuesIn;

INSTANTIATE_TEST_SUITE_P(Easy, AndFromRegister, ValuesIn(easy_from_register));
INSTANTIATE_TEST_SUITE_P(Easy, AndImmediateValue, ValuesIn(easy_imm_d8));
INSTANTIATE_TEST_SUITE_P(Easy, AndHLRelative, ValuesIn(easy_hl_rel));
INSTANTIATE_TEST_SUITE_P(Registers, AndFromRegister,
                         ValuesIn(register_from_register));

} // namespace cpu_tests
