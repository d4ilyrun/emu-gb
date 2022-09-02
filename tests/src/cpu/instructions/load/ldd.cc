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

#include "ld.hxx"

namespace cpu_tests
{

class LDI : public InstructionTest
{
  public:
    LDI() : InstructionTest(1) {}

    void SetUp() override
    {
        InstructionTest::SetUp();
        cpu.registers.h = MSB(address);
        cpu.registers.l = LSB(address);
    }

    void TearDown() override
    {
        ASSERT_EQ(read_memory(address), cpu.registers.a);
        ASSERT_EQ(read_register_16bit(REG_HL), address - 1);
    }

  protected:
    const u16 address = 0x1234;
};

TEST_F(LDI, ToHL)
{
    const u8 instruction[1] = {0x32};
    cpu.registers.a = 0x42;
    Load((void *)instruction);
    execute_instruction();
}

TEST_F(LDI, FromHL)
{
    const u8 instruction[1] = {0x3A};
    write_memory(address, 0x42);
    Load((void *)instruction);
    execute_instruction();
}

} // namespace cpu_tests
