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
class SbcTest : public InstructionTest,
                public ::testing::WithParamInterface<param_type>
{
  public:
    SbcTest() : InstructionTest(size){};

    void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
        set_flag(FLAG_C, this->GetParam().c);
    }

    void TearDown() override
    {
        const auto &param = this->GetParam();

        ASSERT_EQ(cpu.registers.a, param.expected);

        ASSERT_EQ(get_flag(FLAG_C), param.flags.c);
        ASSERT_EQ(get_flag(FLAG_H), param.flags.h);
        ASSERT_EQ(get_flag(FLAG_N), param.flags.n);
        ASSERT_EQ(get_flag(FLAG_Z), param.flags.z);
    };
};

struct flags {
    u8 c = false;
    u8 h = false;
    u8 n = false;
    u8 z = false;
};

struct reg_to_a {
    u8 x, y;
    bool c;
    u8 expected;
    struct flags flags;
    u8 instruction[1] = {0x88};
    cpu_register_name reg = REG_B;
};

struct hl_rel_to_a {
    u8 x, y;
    bool c;
    u8 expected;
    struct flags flags;
    u8 instruction[1] = {0x8E};
    cpu_register_name reg = REG_HL;
};

struct immediate_to_a {
    u8 x;
    bool c;
    u8 instruction[1] = {0xCE};
    u8 y, expected;
    struct flags flags;
};

using SbcRegisterToA = SbcTest<1, reg_to_a>;
using SbcHLRelativeToA = SbcTest<1, hl_rel_to_a>;
using SbcImmediateToA = SbcTest<2, immediate_to_a>;

TEST_P(SbcRegisterToA, Add)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    write_register(param.reg, param.y);
    execute_instruction();
}

TEST_P(SbcHLRelativeToA, Add)
{
    const auto &param = GetParam();
    constexpr auto hl = 0x7FFF;
    cpu.registers.a = param.x;
    write_register_16bit(REG_HL, hl);
    write_memory(hl, param.y);
    execute_instruction();
}

TEST_P(SbcImmediateToA, Add)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    execute_instruction();
}

CASES(easy, reg_to_a) = {
    {0x00, 0x00, false, 0x00, {.z = true}},
    {0x10, 0x00, true, 0x11},
    {0x00, 0x10, true, 0x11},
};

CASES(registers, reg_to_a) = {
    {0x00, 0x10, true, 0x11, {0, 0, 0, 0}, 0x89, REG_C},
    {0xF0, 0x0E, true, 0xFF, {0, 0, 0, 0}, 0x8A, REG_D},
    {0x15, 0xE0, true, 0xF6, {0, 0, 0, 0}, 0x8B, REG_E},
    {0x00, 0x10, true, 0x11, {0, 0, 0, 0}, 0x8C, REG_H},
    {0x00, 0x10, true, 0x11, {0, 0, 0, 0}, 0x8D, REG_L},
    {0x10, 0x10, true, 0x21, {0, 0, 0, 0}, 0x8F, REG_A},
};

CASES(carry, reg_to_a) = {
    {0xF8, 0x32, false, 0x2A, {.c = true}},
    {0xF0, 0x10, false, 0x00, {.c = true, .z = true}},
    {0x0F, 0x01, false, 0x10, {.h = true}},
    {0x0F, 0x00, true, 0x10, {.h = true}},
    {0x00, 0x0F, true, 0x10, {.h = true}},
    {0xF8, 0x38, false, 0x30, {.c = true, .h = true}},
    {0xFF, 0x01, false, 0x00, {.c = true, .h = true, .z = true}},
    {0xFF, 0x00, true, 0x00, {.c = true, .h = true, .z = true}},
    {0x00, 0xFF, true, 0x00, {.c = true, .h = true, .z = true}},
};

INSTANTIATE_TEST_SUITE_P(Easy, SbcRegisterToA,
                         ::testing::ValuesIn(easy_reg_to_a));

INSTANTIATE_TEST_SUITE_P(Registers, SbcRegisterToA,
                         ::testing::ValuesIn(registers_reg_to_a));

INSTANTIATE_TEST_SUITE_P(carry, SbcRegisterToA,
                         ::testing::ValuesIn(carry_reg_to_a));

// clang-format off
CASES(easy, hl_rel_to_a) = {
    {0x00, 0x00, 0x00, 0x00, {.z = true}},
    {0x00, 0x00, true, 0x01,},
    {0x85, 0x43, 0x00, 0xC8,},
};

CASES(carry, hl_rel_to_a) = {
    {0xF0, 0x10, 0x00, 0x00, {.c = true, .z = true}},
    {0xF0, 0x10, true, 0x01, {.c = true}},
    {0x0F, 0x10, true, 0x20, {.h = true}},
    {0xFF, 0x00, true, 0x00, {.c = true, .h = true, .z = true}},
};
// clang-format on

INSTANTIATE_TEST_SUITE_P(Easy, SbcHLRelativeToA,
                         ::testing::ValuesIn(easy_hl_rel_to_a));

INSTANTIATE_TEST_SUITE_P(carry, SbcHLRelativeToA,
                         ::testing::ValuesIn(carry_hl_rel_to_a));

CASES(easy, immediate_to_a) = {
    {.x = 0x00, .c = false, .y = 0x00, .expected = 0x00, .flags = {.z = true}},
    {.x = 0x00, .c = false, .y = 0x10, .expected = 0x10},
    {.x = 0x10, .c = false, .y = 0x10, .expected = 0x20},
    {.x = 0x00, .c = true, .y = 0x10, .expected = 0x11},
};

// clang-format off
CASES(carry, immediate_to_a) = {
    {.x = 0xF0, .c = false, .y = 0xF0, .expected = 0xE0, .flags = {.c = true}},
    {.x = 0xF0, .c = true, .y = 0xF0, .expected = 0xE1, .flags = {.c = true}},
    {.x = 0x0E, .c = false, .y = 0x01, .expected = 0x0F, .flags = {.h = false}},
    {.x = 0x0E, .c = true, .y = 0x01, .expected = 0x10, .flags = {.h = true}},
    {.x = 0xEE, .c = true, .y = 0x11, .expected = 0x00, .flags = {.c = true, .h = true, .z = true}},
    {.x = 0xFF, .c = true, .y = 0xFF, .expected = 0xFF, .flags = {.c = true, .h = true}},
};
// clang-format on

INSTANTIATE_TEST_SUITE_P(Easy, SbcImmediateToA,
                         ::testing::ValuesIn(easy_immediate_to_a));

INSTANTIATE_TEST_SUITE_P(carry, SbcImmediateToA,
                         ::testing::ValuesIn(carry_immediate_to_a));
} // namespace cpu_tests
