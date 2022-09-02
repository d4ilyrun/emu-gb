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
    u8 n = true;
    u8 z = false;
};

struct reg_to_a {
    u8 x, y;
    bool c;
    u8 expected;
    struct flags flags;
    u8 instruction[1] = {0x98};
    cpu_register_name reg = REG_B;
};

struct hl_rel_to_a {
    u8 x, y;
    bool c;
    u8 expected;
    struct flags flags;
    u8 instruction[1] = {0x9E};
    cpu_register_name reg = REG_HL;
};

struct immediate_to_a {
    u8 x;
    bool c;
    u8 instruction[1] = {0xDE};
    u8 y, expected;
    struct flags flags;
};

using SbcRegisterToA = SbcTest<1, reg_to_a>;
using SbcHLRelativeToA = SbcTest<1, hl_rel_to_a>;
using SbcImmediateToA = SbcTest<2, immediate_to_a>;

TEST_P(SbcRegisterToA, Sub)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    write_register(param.reg, param.y);
    execute_instruction();
}

TEST_P(SbcHLRelativeToA, Sub)
{
    const auto &param = GetParam();
    constexpr auto hl = 0x7FF8;
    cpu.registers.a = param.x;
    write_register_16bit(REG_HL, hl);
    write_memory(hl, param.y);
    execute_instruction();
}

TEST_P(SbcImmediateToA, 0xDE)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    execute_instruction();
}

CASES(easy, reg_to_a) = {
    {0x00, 0x00, false, 0x00, {.z = true}},
    {0x11, 0x00, true, 0x10},
    {0x11, 0x01, false, 0x10},
};

CASES(registers, reg_to_a) = {
    {0x11, 0x00, true, 0x10, {0, 0, 1, 0}, 0x99, REG_C},
    {0xF3, 0x01, true, 0xF1, {0, 0, 1, 0}, 0x9A, REG_D},
    {0xF2, 0xE0, true, 0x11, {0, 0, 1, 0}, 0x9B, REG_E},
    {0x11, 0x00, true, 0x10, {0, 0, 1, 0}, 0x9C, REG_H},
    {0x11, 0x00, true, 0x10, {0, 0, 1, 0}, 0x9D, REG_L},
    {0x11, 0x11, false, 0x00, {0, 0, 1, 1}, 0x9F, REG_A},
};

CASES(borrow, reg_to_a) = {
    {0x08, 0x12, false, 0xF6, {.c = true}},
    {0x10, 0x01, false, 0x0F, {.h = true}},
    {0x10, 0x00, true, 0x0F, {.h = true}},
    {0x10, 0x01, true, 0x0E, {.h = true}},
    {0x00, 0x00, true, 0xFF, {.c = true, .h = true}},
    {0x00, 0x01, false, 0xFF, {.c = true, .h = true}},
    {0x00, 0x01, true, 0xFE, {.c = true, .h = true}},
};

INSTANTIATE_TEST_SUITE_P(Easy, SbcRegisterToA,
                         ::testing::ValuesIn(easy_reg_to_a));

INSTANTIATE_TEST_SUITE_P(Registers, SbcRegisterToA,
                         ::testing::ValuesIn(registers_reg_to_a));

INSTANTIATE_TEST_SUITE_P(borrow, SbcRegisterToA,
                         ::testing::ValuesIn(borrow_reg_to_a));

// clang-format off
CASES(easy, hl_rel_to_a) = {
    {0x00, 0x00, 0x00, 0x00, {.z = true}},
    {0x02, 0x00, true, 0x01,},
    {0x85, 0x43, true, 0x41,},
};

CASES(borrow, hl_rel_to_a) = {
    {0x01, 0x10, true, 0xF0, {.c = true}},
    {0xF0, 0x01, false, 0xEF, {.h = true}},
    {0xF0, 0x00, true, 0xEF, {.h = true}},
    {0x00, 0x00, true, 0xFF, {.c = true, .h = true}},
    {0x00, 0x01, false, 0xFF, {.c = true, .h = true}},
};
// clang-format on

INSTANTIATE_TEST_SUITE_P(Easy, SbcHLRelativeToA,
                         ::testing::ValuesIn(easy_hl_rel_to_a));

INSTANTIATE_TEST_SUITE_P(Borrow, SbcHLRelativeToA,
                         ::testing::ValuesIn(borrow_hl_rel_to_a));

CASES(easy, immediate_to_a) = {
    {.x = 0x00, .c = false, .y = 0x00, .expected = 0x00, .flags = {.z = true}},
    {.x = 0x10, .c = false, .y = 0x10, .expected = 0x00, .flags = {.z = true}},
    {.x = 0x13, .c = true, .y = 0x10, .expected = 0x02},
};

// clang-format off
CASES(borrow, immediate_to_a) = {
    {.x = 0xF0, .c = true, .y = 0x00, .expected = 0xEF, .flags = {.h = true}},
    {.x = 0xF0, .c = false, .y = 0x01, .expected = 0xEF, .flags = {.h = true}},
    {.x = 0x0E, .c = true, .y = 0x10, .expected = 0xFD, .flags = {.c = true}},
    {.x = 0x00, .c = true, .y = 0x00, .expected = 0xFF, .flags = {.c = true, .h = true}},
    {.x = 0x00, .c = false, .y = 0x01, .expected = 0xFF, .flags = {.c = true, .h = true}},
    {.x = 0xFF, .c = true, .y = 0xFF, .expected = 0xFF, .flags = {.c = true, .h = true}},
};

CASES(blargg, immediate_to_a) = {
    {.x = 0x00, .c = false, .y = 0x00, .expected = 0x00},
    {.x = 0x00, .c = false, .y = 0x01, .expected = 0xFF, .flags = {.c = true, .h = true}},
    {.x = 0x00, .c = false, .y = 0x0F, .expected = 0xF1, .flags = {.c = true}},
    {.x = 0x00, .c = false, .y = 0x10, .expected = 0xF0, .flags = {.c = true}},
    {.x = 0x00, .c = false, .y = 0x1F, .expected = 0xE1, .flags = {.c = true}},
    {.x = 0x00, .c = false, .y = 0x7F, .expected = 0x81, .flags = {.c = true}},
    {.x = 0x00, .c = false, .y = 0x80, .expected = 0x80, .flags = {.c = true}},
    {.x = 0x00, .c = false, .y = 0xF0, .expected = 0xF0, .flags = {.c = true}},
    {.x = 0x00, .c = false, .y = 0xFF, .expected = 0xF0, .flags = {.c = true}},
};
// clang-format on

INSTANTIATE_TEST_SUITE_P(Easy, SbcImmediateToA,
                         ::testing::ValuesIn(easy_immediate_to_a));

INSTANTIATE_TEST_SUITE_P(Borrow, SbcImmediateToA,
                         ::testing::ValuesIn(borrow_immediate_to_a));
} // namespace cpu_tests
