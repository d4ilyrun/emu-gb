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
class CpTest : public InstructionTest,
               public ::testing::WithParamInterface<param_type>
{
  public:
    CpTest() : InstructionTest(size){};

    void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
    }

    void TearDown() override
    {
        const auto &param = this->GetParam();

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
    struct flags flags;
    u8 instruction[1] = {0xB8};
    cpu_register_name reg = REG_B;
};

struct imm_d8 {
    u8 x;
    u8 instruction[1] = {0xFE};
    u8 y;
    struct flags flags;
};

struct hl_rel {
    u8 x, y;
    struct flags flags;
    u8 instruction[1] = {0xBE};
};

using CpFromRegister = CpTest<1, from_register>;
using CpImmediateValue = CpTest<2, imm_d8>;
using CpHLRelative = CpTest<1, hl_rel>;

TEST_P(CpFromRegister, Sub)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    write_register(param.reg, param.y);
    execute_instruction();
}

TEST_P(CpImmediateValue, Sub)
{
    const auto &param = GetParam();
    cpu.registers.a = param.x;
    execute_instruction();
}

TEST_P(CpHLRelative, Sub)
{
    const auto &param = GetParam();
    const u16 address = 0x7FFF;
    cpu.registers.a = param.x;
    write_register_16bit(REG_HL, address);
    write_memory(address, param.y);
    execute_instruction();
}

CASES(easy, from_register) = {
    {0x00, 0x00, {.z = true}},
    {0xFE, 0xFE, {.z = true}},
    {0xFF, 0x01},
    {0xFF, 0x11},
};

CASES(register, from_register) = {
    {0xFF, 0x01, {false, false, true, false}, 0xB9, REG_C},
    {0xFF, 0x01, {false, false, true, false}, 0xBA, REG_D},
    {0xFF, 0x01, {false, false, true, false}, 0xBB, REG_E},
    {0xFF, 0x01, {false, false, true, false}, 0xBC, REG_H},
    {0xFF, 0x01, {false, false, true, false}, 0xBD, REG_L},
    {0xFF, 0xFF, {false, false, true, true}, 0xBF, REG_A},
};

CASES(borrow, from_register) = {
    {0xF0, 0x01, {.h = true}},
    {0x00, 0x10, {.c = true}},
    {0x00, 0x01, {.c = true, .h = true}},
};

CASES(easy, imm_d8) = {
    {0x00, {0xFE}, 0x00, {.z = true}},
    {0x0E, {0xFE}, 0x01},
    {0xFE, {0xFE}, 0x11},
};

CASES(borrow, imm_d8) = {
    {0xF0, {0xFE}, 0x01, {.h = true}},
    {0x00, {0xFE}, 0x10, {.c = true}},
    {0x00, {0xFE}, 0x01, {.c = true, .h = true}},
};

CASES(easy, hl_rel) = {
    {0x00, 0x00, {.z = true}},
    {0xFE, 0xFE, {.z = true}},
    {0xFF, 0x01},
    {0xFF, 0x11},
};

CASES(borrow, hl_rel) = {
    {0xF0, 0x01, {.h = true}},
    {0x00, 0x10, {.c = true}},
    {0x00, 0x01, {.c = true, .h = true}},
};

using ::testing::ValuesIn;

INSTANTIATE_TEST_SUITE_P(Easy, CpFromRegister, ValuesIn(easy_from_register));
INSTANTIATE_TEST_SUITE_P(Registers, CpFromRegister,
                         ValuesIn(register_from_register));
INSTANTIATE_TEST_SUITE_P(Borrow, CpFromRegister,
                         ValuesIn(borrow_from_register));

INSTANTIATE_TEST_SUITE_P(Easy, CpImmediateValue, ValuesIn(easy_imm_d8));
INSTANTIATE_TEST_SUITE_P(Borrow, CpImmediateValue, ValuesIn(borrow_imm_d8));

INSTANTIATE_TEST_SUITE_P(Easy, CpHLRelative, ValuesIn(easy_hl_rel));
INSTANTIATE_TEST_SUITE_P(Borrow, CpHLRelative, ValuesIn(borrow_hl_rel));

} // namespace cpu_tests
