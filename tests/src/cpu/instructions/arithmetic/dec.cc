
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
    u8 n = true;
    u8 z = false;
};

struct dec_param {
    u8 value;
    cpu_register_name out = REG_B;
    u8 instruction[1] = {0x05};
    struct flags flags;
    u8 msb = 0x0;
};

class Dec : public InstructionTest,
            public ::testing::WithParamInterface<dec_param>
{
  public:
    Dec() : InstructionTest(1){};

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
            ASSERT_EQ(read_register(param.out), (u8)(param.value - 1));
        } else if (param.out != REG_ERR) {
            const u16 value_16bit = (param.msb << 8) | param.value;
            ASSERT_EQ(read_register_16bit(param.out), (u16)(value_16bit - 1));
        } else {
            const auto &hl = read_register_16bit(REG_HL);
            ASSERT_EQ(read_memory(hl), (u8)(param.value - 1));
        }

        ASSERT_EQ(get_flag(FLAG_C), param.flags.c);
        ASSERT_EQ(get_flag(FLAG_H), param.flags.h);
        ASSERT_EQ(get_flag(FLAG_N), param.flags.n);
        ASSERT_EQ(get_flag(FLAG_Z), param.flags.z);
    };
};

TEST_P(Dec, Dec)
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

CASES(bit_8, dec_param) = {
    {0x13},
    {.value = 0x01, .flags = {.z = true}},
    {.value = 0xF0, .flags = {.h = true}},
    {.value = 0x00, .flags = {.h = true}},
};

CASES(registers, dec_param) = {
    {0xFF, REG_C, {0x0D}}, {0xFF, REG_D, {0x15}}, {0xFF, REG_E, {0x1D}},
    {0xFF, REG_H, {0x25}}, {0xFF, REG_L, {0x2D}}, {0xFF, REG_A, {0x3D}},
};

// N flag should not be modified, so stays unset
CASES(bit_16, dec_param) = {
    {0x13, REG_BC, {0x0B}, {.n = false}, 0x34},
    {0x01, REG_DE, {0x1B}, {.n = false}, 0x00}, // Should not carry
    {0x00, REG_HL, {0x2B}, {.n = false}, 0x01}, // Should not zero
    {0x00, REG_SP, {0x3B}, {.n = false}, 0x10}, // Should not half-carry
};

CASES(relative, dec_param) = {{0xFF, REG_ERR, {0x35}}};

using ::testing::ValuesIn;

INSTANTIATE_TEST_SUITE_P(Registers, Dec, ValuesIn(registers_dec_param));
INSTANTIATE_TEST_SUITE_P(Relative, Dec, ValuesIn(relative_dec_param));
INSTANTIATE_TEST_SUITE_P(Bit8, Dec, ValuesIn(bit_8_dec_param));
INSTANTIATE_TEST_SUITE_P(Bit16, Dec, ValuesIn(bit_16_dec_param));

} // namespace cpu_tests
