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

namespace arithmetic_tests
{

struct flags {
    u8 c = false;
    u8 h = false;
    u8 n = false;
    u8 z = false;
};

struct s8_to_sp {
    u16 x;
    u8 instruction[2];
    u16 expected;
    struct flags flags;
};

class LdArithmetic : public InstructionTest,
                     public ::testing::WithParamInterface<s8_to_sp>
{
  public:
    LdArithmetic() : InstructionTest(2){};

    void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
    }

    void TearDown() override
    {
        const auto &param = this->GetParam();

        ASSERT_EQ(read_register_16bit(REG_HL), param.expected);

        ASSERT_EQ(get_flag(FLAG_C), param.flags.c);
        ASSERT_EQ(get_flag(FLAG_H), param.flags.h);
        ASSERT_EQ(get_flag(FLAG_N), param.flags.n);
        ASSERT_EQ(get_flag(FLAG_Z), param.flags.z);
    };
};

TEST_P(LdArithmetic, AddSigned8bitToSP)
{
    const auto &param = GetParam();
    cpu.registers.sp = param.x;
    execute_instruction();
}

CASES(positive, s8_to_sp) = {
    {0x0000, {0xF8, 0x00}, 0x0000}, // Z not set by 0xE8
    {0x0000, {0xF8, 0x11}, 0x0011}, {0xFF00, {0xF8, 0x11}, 0xFF11},
    {0xA130, {0xF8, 0x11}, 0xF141}, {0x1F00, {0xF8, 0x00}, 0x1F00},
};

// H from bit 3, C from bit 7 (flags from low byte op)
CASES(positive_carry, s8_to_sp) = {
    {0x000F, {0xF8, 0x01}, 0x0010, {.h = true}},
    {0x00F0, {0xF8, 0x10}, 0x0010, {.c = true}},
    {0x00FF, {0xF8, 0x11}, 0x0110, {.c = true, .h = true}},
    {0x00FF, {0xF8, 0x01}, 0x0100, {.c = true, .h = true}},
    {0x0F0F, {0xF8, 0x01}, 0x0010, {.h = true}},
    {0x0FF0, {0xF8, 0x10}, 0x1000, {.c = true}},
    {0xFFF0, {0xF8, 0x10}, 0x0000, {.c = true}},
    {0x0FFF, {0xF8, 0x01}, 0x1000, {.c = true, .h = true}},
};

CASES(negative, s8_to_sp) = {
    {0x000F, {0xF8, 0xFF}, 0x000E}, {0x1A0F, {0xF8, 0xFF}, 0x1A0E},
    {0x003F, {0xF8, 0x81}, 0x00C0}, // negative overflow
    {0x1A3F, {0xF8, 0x81}, 0x1BC0}, // negative overflow
    {0x103F, {0xF8, 0x81}, 0x00C0}, // negative overflow
    {0xFF00, {0xF8, 0xFF}, 0xFEFF},
};

CASES(negative_carry, s8_to_sp) = {
    {0xFF0F, {0xF8, 0xFF}, 0xFF0E, {.h = true}},
    {0xFFF0, {0xF8, 0xFF}, 0xFFEF, {.c = true}},
    {0x00FF, {0xF8, 0xFF}, 0x00FE, {.c = true, .h = true}},
    {0xFFFF, {0xF8, 0xFF}, 0xFFFE, {.c = true, .h = true}},
};

INSTANTIATE_TEST_SUITE_P(Positive, LdArithmetic,
                         ::testing::ValuesIn(positive_s8_to_sp));

INSTANTIATE_TEST_SUITE_P(PositiveWithCarry, LdArithmetic,
                         ::testing::ValuesIn(positive_carry_s8_to_sp));

INSTANTIATE_TEST_SUITE_P(Negative, LdArithmetic,
                         ::testing::ValuesIn(negative_s8_to_sp));

INSTANTIATE_TEST_SUITE_P(NegativeWithCarryarry, LdArithmetic,
                         ::testing::ValuesIn(negative_carry_s8_to_sp));

} // namespace arithmetic_tests
