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

#include "ld.hxx"

namespace cpu_tests
{

struct ld_imm_params {
    u8 instruction[2];
    u8 *out; // cpu register
    u8 expected;
};

class LoadImmediate : public LoadInstructionTest<2 * sizeof(u8), ld_imm_params>
{
  public:
    LoadImmediate() : LoadInstructionTest() {}
    static std::vector<ld_imm_params> cases;
};

std::vector<ld_imm_params> LoadImmediate::cases{
    {{0b00111110, 0xFF}, &cpu.registers.a, 0xFF}, // A
    {{0b00000110, 0x42}, &cpu.registers.b, 0x42}, // B
    {{0b00001110, 0x00}, &cpu.registers.c, 0x00}, // C
    {{0b00010110, 0x0F}, &cpu.registers.d, 0x0F}, // D
    {{0b00011110, 0xF0}, &cpu.registers.e, 0xF0}, // E
    {{0b00100110, 0x08}, &cpu.registers.h, 0x08}, // H
    {{0b00101110, 0xA5}, &cpu.registers.l, 0xA5}, // L
};

TEST_P(LoadImmediate, ImmediateValueToRegister)
{
    execute_instruction();
}

INSTANTIATE_TEST_SUITE_P(LoadImmediate, LoadImmediate,
                         testing::ValuesIn(LoadImmediate::cases));

}; // namespace cpu_tests
