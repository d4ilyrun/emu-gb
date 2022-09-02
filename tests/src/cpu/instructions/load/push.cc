// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <cpu/cpu.h>
#include <cpu/instruction.h>
#include <utils/macro.h>
}

#include "../instruction.hxx"

namespace cpu_tests
{

struct push_param {
    u16 data;
    u8 instruction = 0xC5;
    cpu_register_name reg = REG_BC;
};

class Push : public InstructionTest,
             public ::testing::WithParamInterface<push_param>
{
  public:
    Push() : InstructionTest(1) {}

    void SetUp() override
    {
        InstructionTest::SetUp();
        cpu.registers.sp = start_sp_;
        Load((void *)&this->GetParam().instruction);
    }

    void TearDown() override
    {
        InstructionTest::TearDown();
        ASSERT_EQ(cpu.registers.sp, start_sp_ - 2);
        ASSERT_EQ(read_memory_16bit(cpu.registers.sp), GetParam().data);
    }

  private:
    const u16 start_sp_ = 0x8000;
};

TEST_P(Push, Push)
{
    const auto &param = GetParam();
    write_register_16bit(param.reg, param.data);
    execute_instruction();
}

CASES(registers, push_param) = {{0x0000, 0xC5, REG_BC},
                                {0xFF00, 0xD5, REG_DE},
                                {0x00FF, 0xE5, REG_HL},
                                {0xFFFF, 0xF5, REG_AF}};

INSTANTIATE_TEST_SUITE_P(Registers, Push,
                         ::testing::ValuesIn(registers_push_param));

} // namespace cpu_tests
