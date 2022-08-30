// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include <CPU/cpu.h>
#include <CPU/instruction.h>
#include <utils/macro.h>
}

#include "../instruction.hxx"

namespace cpu_tests
{

struct pop_param {
    u16 data;
    u8 instruction = 0xC5;
    cpu_register_name reg = REG_BC;
};

class Pop : public InstructionTest,
            public ::testing::WithParamInterface<pop_param>
{
  public:
    Pop() : InstructionTest(1) {}

    void SetUp() override
    {
        InstructionTest::SetUp();
        cpu.registers.sp = start_sp_;
        Load((void *)&this->GetParam().instruction);
    }

    void TearDown() override
    {
        InstructionTest::TearDown();
        ASSERT_EQ(cpu.registers.sp, start_sp_ + 2);

        const auto &param = GetParam();

        if (param.reg == REG_AF) {
            ASSERT_EQ(read_register_16bit(param.reg), param.data & 0xFFF0);
        } else {
            ASSERT_EQ(read_register_16bit(param.reg), param.data);
        }
    }

  protected:
    const u16 start_sp_ = 0x2000;
};

TEST_P(Pop, Pop)
{
    const auto &param = GetParam();
    write_memory_16bit(this->start_sp_, param.data);
    execute_instruction();
}

CASES(registers, pop_param) = {{0x0000, 0xC1, REG_BC},
                               {0xFF00, 0xD1, REG_DE},
                               {0x00FF, 0xE1, REG_HL},
                               {0xFFF0, 0xF1, REG_AF},
                               {0xFFFF, 0xF1, REG_AF}};

INSTANTIATE_TEST_SUITE_P(Registers, Pop,
                         ::testing::ValuesIn(registers_pop_param));

} // namespace cpu_tests
