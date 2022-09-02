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

using testing::ValuesIn;

template <uint size, typename param_type>
class DaaTest : public InstructionTest,
                public ::testing::WithParamInterface<param_type>
{
  public:
    DaaTest() : InstructionTest(size){};

    void SetUp() override
    {
        InstructionTest::SetUp();

        const auto &param = this->GetParam();
        const u8 opcode[1] = {0x27};

        Load((void *)opcode);
        cpu.registers.a = param.a;

        set_flag(FLAG_C, param.c);
        set_flag(FLAG_H, param.h);
    }

    void TearDown() override
    {
        const auto &param = this->GetParam();

        ASSERT_EQ(cpu.registers.a, param.expected);

        ASSERT_EQ(get_flag(FLAG_C), param.flags.c | param.c);
        ASSERT_EQ(get_flag(FLAG_Z), param.flags.z);
        ASSERT_EQ(get_flag(FLAG_H), false);
    };
};

struct flags {
    u8 c = false;
    u8 z = false;
};

struct daa {
    u8 a, expected;
    struct flags flags;
    u8 c = false;
    u8 h = false;
};

using DaaAdd = DaaTest<1, daa>;
using DaaSub = DaaTest<1, daa>;

TEST_P(DaaAdd, DaaAfterAdd)
{
    set_flag(FLAG_N, false);
    execute_instruction();
    ASSERT_EQ(get_flag(FLAG_N), false);
}

TEST_P(DaaSub, DaaAfterSub)
{
    set_flag(FLAG_N, true);
    execute_instruction();
    ASSERT_EQ(get_flag(FLAG_N), true);
}

CASES(easy, daa) = {
    {0x00, 0x00, {.z = true}}, {0x09, 0x09}, {0x0A, 0x10}, {0x10, 0x10},
    {0xF0, 0x50, {.c = true}}, {0x1A, 0x20},
};

// clang-format off
CASES(carry, daa) = {
    {0x00, 0x60, {.c = true}, true},
    {0x20, 0x80, {.c = true}, true},
    {0x10, 0x16, {}, false, true},
    {0x12, 0x18, {}, false, true},
    {0x1F, 0x85, {.c = true}, true, true},
};
// clang-format on

INSTANTIATE_TEST_SUITE_P(Easy, DaaAdd, ValuesIn(easy_daa));
INSTANTIATE_TEST_SUITE_P(Carry, DaaAdd, ValuesIn(carry_daa));

CASES(easy_sub, daa) = {
    {0x00, 0x00, {.z = true}},
    {0x09, 0x09},
    {0x0A, 0x0A},
    {0x10, 0x10},
    {0xF0, 0xF0},
    {0x1A, 0x1A},
};

CASES(borrow, daa) = {
    {0x0F, 0x09, {}, false, true},
    {0x1F, 0x19, {}, false, true},
    {0xF0, 0x90, {}, true, false},
    {0x50, 0xF0, {}, true, false},
    {0xFF, 0x99, {}, true, true},
    {0x67, 0x01, {}, true, true},
    {0x60, 0x00, {.z = true}, true, false},
    {0x06, 0x00, {.z = true}, false, true},
};

INSTANTIATE_TEST_SUITE_P(Easy, DaaSub, ValuesIn(easy_sub_daa));
INSTANTIATE_TEST_SUITE_P(Borrow, DaaSub, ValuesIn(borrow_daa));

} // namespace cpu_tests
