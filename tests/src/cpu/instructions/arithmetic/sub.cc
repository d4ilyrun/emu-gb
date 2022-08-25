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

using ::testing::ValuesIn;

namespace cpu_tests
{

template <uint size, typename param_type, cpu_register_name out = REG_A>
class SubTest : public InstructionTest,
                public ::testing::WithParamInterface<param_type>
{
  public:
    SubTest() : InstructionTest(size){};

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
    u8 n = true;
    u8 z = false;
};

struct from_register {
    u8 x, y;
    u8 expected;
    struct flags flags;
    u8 instruction[1] = {0x90};
    cpu_register_name reg = REG_B;
};

struct imm_d8 {
    u8 x;
    u8 instruction[2];
    u8 expected;
    struct flags flags;
};

struct hl_rel {
    u8 x, y;
    u8 expected;
    struct flags flags;
    u8 instruction[2] = {0x96};
};

using SubFromRegister = SubTest<1, from_register>;
using SubImmediateValue = SubTest<2, imm_d8>;
using SubHLRelative = SubTest<1, hl_rel>;

TEST_P(SubFromRegister, Sub)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    write_register(param.reg, param.y);
    execute_instruction();
}

TEST_P(SubImmediateValue, Sub)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    execute_instruction();
}

TEST_P(SubHLRelative, Sub)
{
    const auto &param = GetParam();
    const u16 address = 0x7FFF;
    cpu.registers.a = param.x;
    write_register_16bit(REG_HL, address);
    write_memory(address, param.y);
    execute_instruction();
}

CASES(easy, from_register) = {
    {0x00, 0x00, 0x00, {.z = true}},
    {0xFE, 0xFE, 0x00, {.z = true}},
    {0xFF, 0x01, 0xFE},
    {0xFF, 0x11, 0xEE},
};

CASES(registers, from_register) = {
    {0xFF, 0x01, 0xFE, {false, false, true, false}, 0x91, REG_C},
    {0xFF, 0x01, 0xFE, {false, false, true, false}, 0x92, REG_D},
    {0xFF, 0x01, 0xFE, {false, false, true, false}, 0x93, REG_E},
    {0xFF, 0x01, 0xFE, {false, false, true, false}, 0x94, REG_H},
    {0xFF, 0x01, 0xFE, {false, false, true, false}, 0x95, REG_L},
    {0xFF, 0xFF, 0x00, {false, false, true, true}, 0x96, REG_A},
};

CASES(borrow, from_register) = {
    {0xF0, 0x01, 0xEF, {.h = true}},
    {0x00, 0x10, 0xF0, {.c = true}},
    {0x00, 0x01, 0xFF, {.c = true, .h = true}},
};

INSTANTIATE_TEST_SUITE_P(Easy, SubFromRegister, ValuesIn(easy_from_register));
INSTANTIATE_TEST_SUITE_P(Registers, SubFromRegister,
                         ValuesIn(registers_from_register));
INSTANTIATE_TEST_SUITE_P(Borrow, SubFromRegister,
                         ValuesIn(borrow_from_register));

CASES(easy, imm_d8) = {
    {0x00, {0xD6, 0x00}, 0x00, {.z = true}},
    {0x0E, {0xD6, 0x01}, 0x0D},
    {0xFE, {0xD6, 0x11}, 0xED},
};

CASES(borrow, imm_d8) = {
    {0xF0, {0xD6, 0x01}, 0xEF, {.h = true}},
    {0x00, {0xD6, 0x10}, 0xF0, {.c = true}},
    {0x00, {0xD6, 0x01}, 0xFF, {.c = true, .h = true}},
};

INSTANTIATE_TEST_SUITE_P(Easy, SubImmediateValue, ValuesIn(easy_imm_d8));
INSTANTIATE_TEST_SUITE_P(Borrow, SubImmediateValue, ValuesIn(borrow_imm_d8));

CASES(easy, hl_rel) = {
    {0x00, 0x00, 0x00, {.z = true}},
    {0xFE, 0xFE, 0x00, {.z = true}},
    {0xFF, 0x01, 0xFE},
    {0xFF, 0x11, 0xEE},
};

CASES(borrow, hl_rel) = {
    {0xF0, 0x01, 0xEF, {.h = true}},
    {0x00, 0x10, 0xF0, {.c = true}},
    {0x00, 0x01, 0xFF, {.c = true, .h = true}},
};

INSTANTIATE_TEST_SUITE_P(Easy, SubHLRelative, ValuesIn(easy_hl_rel));
INSTANTIATE_TEST_SUITE_P(Borrow, SubHLRelative, ValuesIn(borrow_hl_rel));

} // namespace cpu_tests
