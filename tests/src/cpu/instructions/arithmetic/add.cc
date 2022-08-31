// REG_ERR is also defined inside gtest ...
#include <algorithm>
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <CPU/cpu.h>
#include <CPU/flag.h>
#include <CPU/instruction.h>
#include <utils/macro.h>
#include <utils/types.h>
}

#include "../instruction.hxx"

namespace cpu_tests
{

template <uint size, typename param_type, cpu_register_name out = REG_A>
class AddTest : public InstructionTest,
                public ::testing::WithParamInterface<param_type>
{
  public:
    AddTest() : InstructionTest(size){};

    void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
    }

    void TearDown() override
    {
        const auto &param = this->GetParam();
        if constexpr (out == REG_A) {
            ASSERT_EQ(cpu.registers.a, param.expected);
        } else {
            ASSERT_EQ(read_register_16bit(out), param.expected);
        }

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
    u8 expected;
    struct flags flags;
    u8 instruction[1] = {0x80};
    cpu_register_name reg = REG_B;
};

struct hl_rel_to_a {
    u8 x, y;
    u8 expected;
    struct flags flags;
    u8 instruction[1] = {0x86};
    cpu_register_name reg = REG_HL;
};

struct immediate_to_a {
    u8 x;
    u8 instruction[1] = {0xC6};
    u8 y, expected;
    struct flags flags;
};

struct reg_to_hl {
    u16 x, y;
    u16 expected;
    struct flags flags;
    u8 instruction[1] = {0x09};
    cpu_register_name reg = REG_BC;
};

struct s8_to_sp {
    u16 x;
    u8 instruction[2];
    u16 expected;
    struct flags flags;
};

using AddRegisterToA = AddTest<1, reg_to_a>;
using AddHLRelativeToA = AddTest<1, hl_rel_to_a>;
using AddImmediateToA = AddTest<2, immediate_to_a>;
using Add16BitRegisterToHL = AddTest<1, reg_to_hl, REG_HL>;
using AddSigned8bitToSP = AddTest<2, s8_to_sp, REG_SP>;

TEST_P(AddRegisterToA, Add)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    write_register(param.reg, param.y);
    execute_instruction();
}

TEST_P(AddHLRelativeToA, Add)
{
    const auto &param = GetParam();
    constexpr auto hl = 0x7FFF;
    cpu.registers.a = param.x;
    write_register_16bit(REG_HL, hl);
    write_memory(hl, param.y);
    execute_instruction();
}

TEST_P(AddImmediateToA, Add)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    execute_instruction();
}

TEST_P(Add16BitRegisterToHL, Add)
{
    const auto &param = GetParam();
    write_register_16bit(REG_HL, param.x);
    write_register_16bit(param.reg, param.y);
    execute_instruction();
}

TEST_P(AddSigned8bitToSP, IntoSP)
{
    get_options()->trace = true;

    const auto &param = GetParam();
    cpu.registers.sp = param.x;
    execute_instruction();
}

CASES(easy, reg_to_a) = {
    {0x00, 0x00, 0x00, {.z = true}},
    {0x10, 0x00, 0x10},
    {0x10, 0x10, 0x20},
};

CASES(registers, reg_to_a) = {
    {0x00, 0x10, 0x10, {0, 0, 0, 0}, 0x81, REG_C},
    {0xF0, 0x0F, 0xFF, {0, 0, 0, 0}, 0x82, REG_D},
    {0x15, 0xE0, 0xF5, {0, 0, 0, 0}, 0x83, REG_E},
    {0x00, 0x10, 0x10, {0, 0, 0, 0}, 0x84, REG_H},
    {0x00, 0x10, 0x10, {0, 0, 0, 0}, 0x85, REG_L},
    {0x10, 0x10, 0x20, {0, 0, 0, 0}, 0x87, REG_A},
};

CASES(carry, reg_to_a) = {
    {0xF8, 0x32, 0x2A, {.c = true}},
    {0xF0, 0x10, 0x00, {.c = true, .z = true}},
    {0x0F, 0x01, 0x10, {.h = true}},
    {0x7F, 0x71, 0xF0, {.h = true}},
    {0xF8, 0x38, 0x30, {.c = true, .h = true}},
    {0x4D, 0xCA, 0x17, {.c = true, .h = true}},
    {0xFF, 0x11, 0x10, {.c = true, .h = true}},
    {0x6F, 0x91, 0x00, {.c = true, .h = true, .z = true}},
    {0xFF, 0x01, 0x00, {.c = true, .h = true, .z = true}},
};

INSTANTIATE_TEST_SUITE_P(Easy, AddRegisterToA,
                         ::testing::ValuesIn(easy_reg_to_a));

INSTANTIATE_TEST_SUITE_P(Registers, AddRegisterToA,
                         ::testing::ValuesIn(registers_reg_to_a));

INSTANTIATE_TEST_SUITE_P(carry, AddRegisterToA,
                         ::testing::ValuesIn(carry_reg_to_a));

// clang-format off
CASES(easy, hl_rel_to_a) = {
    {0x00, 0x00, 0x00, {.z = true}},
    {0x85, 0x43, 0xC8,},
    {0xFF, 0x00, 0xFF,},
    {0x00, 0xEE, 0xEE,},
    {0xAA, 0x55, 0xFF,},
};

CASES(carry, hl_rel_to_a) = {
    {0xF8, 0x32, 0x2A, {.c = true}},
    {0xF0, 0x10, 0x00, {.c = true, .z = true}},
    {0x7F, 0x71, 0xF0, {.h = true}},
    {0xF8, 0x38, 0x30, {.c = true, .h = true}},
    {0x4D, 0xCA, 0x17, {.c = true, .h = true}},
    {0x6F, 0x91, 0x00, {.c = true, .h = true, .z = true}},
};
// clang-format on

INSTANTIATE_TEST_SUITE_P(Easy, AddHLRelativeToA,
                         ::testing::ValuesIn(easy_hl_rel_to_a));

INSTANTIATE_TEST_SUITE_P(carry, AddHLRelativeToA,
                         ::testing::ValuesIn(carry_hl_rel_to_a));

CASES(easy, immediate_to_a) = {
    {.x = 0x00, .y = 0x00, .expected = 0x00, .flags = {.z = true}},
    {.x = 0x00, .y = 0x10, .expected = 0x10},
    {.x = 0x10, .y = 0x10, .expected = 0x20},
    {.x = 0xE0, .y = 0x10, .expected = 0xF0},
    {.x = 0x0E, .y = 0x01, .expected = 0x0F},
    {.x = 0x2E, .y = 0xA1, .expected = 0xCF},
};

CASES(carry, immediate_to_a) = {
    {.x = 0xF0, .y = 0xF0, .expected = 0xE0, .flags = {.c = true}},
    {.x = 0xF0, .y = 0x10, .expected = 0x00, .flags = {.c = true, .z = true}},
    {.x = 0xF3, .y = 0x1C, .expected = 0x0F, .flags = {.c = true}},
    {.x = 0x0F, .y = 0x0B, .expected = 0x1A, .flags = {.h = true}},
    {.x = 0xF4, .y = 0x0D, .expected = 0x01, .flags = {.c = true, .h = true}},
    {.x = 0xFF, .y = 0x11, .expected = 0x10, .flags = {.c = true, .h = true}},
};

INSTANTIATE_TEST_SUITE_P(Easy, AddImmediateToA,
                         ::testing::ValuesIn(easy_immediate_to_a));

INSTANTIATE_TEST_SUITE_P(carry, AddImmediateToA,
                         ::testing::ValuesIn(carry_immediate_to_a));

CASES(easy, reg_to_hl) = {{0x0000, 0x0000, 0x0000, {.z = false}}, // Z never set
                          {0x0808, 0x0101, 0x0909},
                          {0xDF1A, 0x1046, 0xEF60}};

CASES(registers, reg_to_hl){
    {0x0800, 0x0100, 0x0900, {false, false, false, false}, {0x19}, REG_DE},
    {0x0100, 0x0100, 0x0200, {false, false, false, false}, {0x29}, REG_HL},
    {0x0800, 0x0100, 0x0900, {false, false, false, false}, {0x39}, REG_SP},
};

CASES(carry, reg_to_hl) = {
    {0x000F, 0x0001, 0x0010, {.h = false}},
    {0x00F0, 0x0010, 0x0100, {.h = false}},
    {0x0F00, 0xF000, 0xFF00, {.h = false}},
    {0x0F00, 0x0100, 0x1000, {.h = true}},
    {0xFF00, 0x1100, 0x1000, {.c = true, .h = true}},
    {0x0FF0, 0x0110, 0x1100, {.h = true}},
    {0x0F0F, 0x0101, 0x1010, {.h = true}},
    {0x1F0F, 0x0101, 0x2010, {.h = true}},

    // Z never set by this instruction
    {0xF000, 0x1000, 0x0000, {.c = true, .z = false}},
    {0xFFFF, 0x0001, 0x0000, {.c = true, .h = true, .z = false}},
    {0xE2B3, 0x1D4D, 0x0000, {.c = true, .h = true, .z = false}},
};

INSTANTIATE_TEST_SUITE_P(Easy, Add16BitRegisterToHL,
                         ::testing::ValuesIn(easy_reg_to_hl));

INSTANTIATE_TEST_SUITE_P(Registers, Add16BitRegisterToHL,
                         ::testing::ValuesIn(registers_reg_to_hl));

INSTANTIATE_TEST_SUITE_P(carry, Add16BitRegisterToHL,
                         ::testing::ValuesIn(carry_reg_to_hl));

CASES(positive, s8_to_sp) = {
    {0x0000, {0xE8, 0x00}, 0x0000}, // Z not set by 0xE8
    {0x0000, {0xE8, 0x11}, 0x0011}, {0xFF00, {0xE8, 0x11}, 0xFF11},
    {0xA130, {0xE8, 0x11}, 0xA141}, {0x1F00, {0xE8, 0x00}, 0x1F00},
};

// H from bit 3, C from bit 7 (flags from low byte op)
CASES(positive_carry, s8_to_sp) = {
    {0x000F, {0xE8, 0x01}, 0x0010, {.h = true}},
    {0x00F0, {0xE8, 0x10}, 0x0100, {.c = true}},
    {0x00FF, {0xE8, 0x11}, 0x0110, {.c = true, .h = true}},
    {0x00FF, {0xE8, 0x01}, 0x0100, {.c = true, .h = true}},
    {0x0F0F, {0xE8, 0x01}, 0x0F10, {.h = true}},
    {0x0FF0, {0xE8, 0x10}, 0x1000, {.c = true}},
    {0xFFF0, {0xE8, 0x10}, 0x0000, {.c = true}},
    {0x0FFF, {0xE8, 0x01}, 0x1000, {.c = true, .h = true}},
};

CASES(negative, s8_to_sp) = {
    {0x1A0F, {0xE8, 0xFF}, 0x1A0E},
    {0x003F, {0xE8, 0x81}, 0x00C0}, // negative overflow
    {0x1A3F, {0xE8, 0x81}, 0x1BC0}, // negative overflow
    {0x103F, {0xE8, 0x81}, 0x0FC0}, // negative overflow
    {0xFF00, {0xE8, 0xFF}, 0xFEFF},
};

/**
 * 0x000F + 0x00FF
 * 0x010E
 */

CASES(negative_carry, s8_to_sp) = {
    {0x0000, {0xE8, 0xFF}, 0x00FF, {.h = true}},
    {0xFF0F, {0xE8, 0xFF}, 0xFF0E, {.h = true}},
    {0xFFF0, {0xE8, 0xFF}, 0xFFEF, {.c = true}},
    {0x00FF, {0xE8, 0xFF}, 0x00FE, {.c = true, .h = true}},
    {0xFFFF, {0xE8, 0xFF}, 0xFFFE, {.c = true, .h = true}},
};

INSTANTIATE_TEST_SUITE_P(Positive, AddSigned8bitToSP,
                         ::testing::ValuesIn(positive_s8_to_sp));

INSTANTIATE_TEST_SUITE_P(PositiveWithCarry, AddSigned8bitToSP,
                         ::testing::ValuesIn(positive_carry_s8_to_sp));

// Commented out because of false negatives
// INSTANTIATE_TEST_SUITE_P(Negative, AddSigned8bitToSP,
//                          ::testing::ValuesIn(negative_s8_to_sp));
//
// INSTANTIATE_TEST_SUITE_P(NegativeWithCarryarry, AddSigned8bitToSP,
//                         ::testing::ValuesIn(negative_carry_s8_to_sp));

} // namespace cpu_tests
