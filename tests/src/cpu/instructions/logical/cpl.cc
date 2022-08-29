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

class CplTest : public InstructionTest, public ::testing::WithParamInterface<u8>
{
  public:
    CplTest() : InstructionTest(1){};

    void SetUp() override
    {
        InstructionTest::SetUp();
        const u8 opcode[1] = {0x2F};
        Load((void *)opcode);
    }

    void TearDown() override
    {
        ASSERT_EQ(cpu.registers.a, GetParam() ^ 0xFF);
        ASSERT_EQ(get_flag(FLAG_H), true);
        ASSERT_EQ(get_flag(FLAG_N), true);
    };
};

TEST_P(CplTest, Cpl)
{
    cpu.registers.a = GetParam();
    execute_instruction();
}

CASES(easy, u8) = {0x00, 0xFF, 0x0F, 0xF0, 0b10101010, 0b01010101};

INSTANTIATE_TEST_SUITE_P(Easy, CplTest, ::testing::ValuesIn(easy_u8));

} // namespace cpu_tests
