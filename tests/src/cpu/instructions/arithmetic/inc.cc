
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

struct flags {
    u8 c = false;
    u8 h = false;
    u8 n = false;
    u8 z = false;
};

struct inc_param {
    u8 value;
    cpu_register_name out = REG_B;
    u8 instruction[1] = {0x04};
    struct flags flags;
    u8 msb = 0x0;
};

class Inc : public InstructionTest,
            public ::testing::WithParamInterface<inc_param>
{
  public:
    Inc() : InstructionTest(1){};

    void SetUp() override
    {
        InstructionTest::SetUp();
        cpu.registers.h = 0x12;
        cpu.registers.l = 0x34;
        Load((void *)this->GetParam().instruction);
    }

    void TearDown() override
    {
        const auto &param = this->GetParam();
        if (!IS_16BIT(param.out)) {
            ASSERT_EQ(read_register(param.out), (u8)(param.value + 1));
        } else if (param.out != REG_ERR) {
            const u16 value_16bit = (param.msb << 8) | param.value;
            ASSERT_EQ(read_register_16bit(param.out), (u16)(value_16bit + 1));
        } else {
            const auto &hl = read_register_16bit(REG_HL);
            ASSERT_EQ(read_memory(hl), (u8)(param.value + 1));
        }

        ASSERT_EQ(get_flag(FLAG_C), param.flags.c);
        ASSERT_EQ(get_flag(FLAG_H), param.flags.h);
        ASSERT_EQ(get_flag(FLAG_N), param.flags.n);
        ASSERT_EQ(get_flag(FLAG_Z), param.flags.z);
    };
};

TEST_P(Inc, Inc)
{
    const auto &param = GetParam();

    if (param.out == REG_ERR) {
        // HL RELATIVE
        write_memory(read_register(REG_HL), param.value);
    } else if (IS_16BIT(param.out)) {
        const u16 value_16bit = (param.msb << 8) | param.value;
        write_register_16bit(param.out, value_16bit);
    } else {
        write_register(param.out, param.value);
    }

    execute_instruction();
}

CASES(bit_8, inc_param) = {
    {0x13},
    {.value = 0xFF, .flags = {.h = true, .z = true}},
    {.value = 0x0F, .flags = {.h = true}},
};

CASES(registers, inc_param) = {
    {0x42, REG_C, {0x0C}}, {0x42, REG_D, {0x14}}, {0x42, REG_E, {0x1C}},
    {0x42, REG_H, {0x24}}, {0x42, REG_L, {0x2C}}, {0x42, REG_A, {0x3C}},
};

CASES(bit_16, inc_param) = {
    {0x13, REG_BC, {0x03}, {}, 0x34},
    {0x00, REG_DE, {0x13}, {}, 0xFF}, // Should not carry
    {0xFF, REG_HL, {0x23}, {}, 0xF1}, // Should not zero nor carry
    {0x00, REG_SP, {0x33}, {}, 0x0F}, // Should not half-carry
    {0x0F, REG_SP, {0x33}, {}, 0xFF}, // Should not half-carry
};

CASES(relative, inc_param) = {{0x00, REG_ERR, {0x34}}};

using ::testing::ValuesIn;

INSTANTIATE_TEST_SUITE_P(Registers, Inc, ValuesIn(registers_inc_param));
INSTANTIATE_TEST_SUITE_P(Relative, Inc, ValuesIn(relative_inc_param));
INSTANTIATE_TEST_SUITE_P(Bit8, Inc, ValuesIn(bit_8_inc_param));
INSTANTIATE_TEST_SUITE_P(Bit16, Inc, ValuesIn(bit_16_inc_param));

} // namespace cpu_tests
