// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

#include <algorithm>

extern "C" {
#include "cpu/cpu.h"
#include "utils/macro.h"
}

namespace cpu_tests
{

template <typename T>
class TestCPURegisters : public ::testing::TestWithParam<T>
{
  public:
    TestCPURegisters() {}

    void SetUp()
    {
        memset(&g_cpu.registers, 0, sizeof(struct cpu_registers));
    }
};

#pragma region tests_8bit

using CPURegisters8bit = TestCPURegisters<u8>;

/**
 * \brief Map a function to each of the CPU's 8bits registers
 */
#define MAP_8BIT_REG(foo)                      \
    do {                                       \
        for (int i = REG_A; i <= REG_L; ++i) { \
            foo(i);                            \
        }                                      \
    } while (0);

#define SET_8BIT_REG(value_) \
    MAP_8BIT_REG([](int i) { write_register((cpu_register_name)i, (value_)); })

/**
 * \brief Compare the content inside the CPU's 8bits registers with a list of
 * values
 */
#define ASSERT_REGISTERS_EQ(expected)                                  \
    do {                                                               \
        auto tmp = expected;                                           \
        MAP_8BIT_REG([&tmp](int i) {                                   \
            u8 got = read_register(static_cast<cpu_register_name>(i)); \
            ASSERT_EQ((tmp)[i], got);                                  \
        });                                                            \
    } while (0);

#define NB_REG 8

// Empty registers (should read 0)
TEST_F(CPURegisters8bit, NullValues)
{
    const u8 null_registers[NB_REG]{0};
    ASSERT_REGISTERS_EQ(null_registers);
}

TEST_P(CPURegisters8bit, Read)
{
    u8 expected[NB_REG] = {0};
    const auto &val = GetParam();

    memset(expected, val, sizeof(expected));
    memset(&g_cpu.registers, val, sizeof(g_cpu.registers));

    ASSERT_REGISTERS_EQ(expected);
}

TEST_P(CPURegisters8bit, Write)
{
    SET_8BIT_REG(GetParam());

    const auto &val = GetParam();
    const auto &reg = g_cpu.registers;

    const auto reg_values = {reg.a, reg.f, reg.b, reg.c,
                             reg.d, reg.e, reg.h, reg.l};

    ASSERT_TRUE(std::all_of(reg_values.begin(), reg_values.end(),
                            [&val](u8 x) { return x == val; }));
}

// Write native 16-bit registers (PC/SP)
// Should write onto the lower byte (0x00??)
TEST_F(CPURegisters8bit, WriteSpecialRegister)
{
    const u8 val = 0x27;

    write_register(REG_PC, val);
    write_register(REG_SP, val | 0x10);

    ASSERT_EQ(g_cpu.registers.pc, val);
    ASSERT_EQ(g_cpu.registers.sp, val | 0x10);
}

// Read native 16-bit registers (PC/SP)
// Should read the lower half only (0x1027 -> 0x27)
TEST_F(CPURegisters8bit, ReadSpecialRegister)
{
    const u8 val = 0x27;

    g_cpu.registers.pc = val | 0x1000;
    g_cpu.registers.sp = val | 0x1010;

    auto pc = read_register(REG_PC);
    auto sp = read_register(REG_SP);

    ASSERT_EQ(pc, val);
    ASSERT_EQ(sp, val | 0x10);
}

// Test cases:
// - null value (0x00)
// - max value (0xFF)
// - "mirror" values (0b1010 0b0101 ; 0b1000 0b0001 ; ...)
INSTANTIATE_TEST_SUITE_P(CPURegisters, CPURegisters8bit,
                         ::testing::Values(0x00, 0xFF, 0x80, 0x01, 0xF0, 0x0F,
                                           0x18, 0x81, 0x5A));

#pragma endregion tests_8bit

#pragma region tests_16bit

using CPURegisters16bit = TestCPURegisters<u16>;

/**
 * \brief Map a function to each of the CPU's 16bits registers
 */
#define MAP_16BIT_REG(foo) \
    do {                   \
        foo(REG_AF);       \
        foo(REG_BC);       \
        foo(REG_DE);       \
        foo(REG_HL);       \
    } while (0);

#define SET_16BIT_REG(value) \
    MAP_16BIT_REG(           \
        [](int i) { write_register_16bit((cpu_register_name)i, value); })

/**
 * \brief Compare the content inside the CPU's 16bits registers with a list of
 * values
 */
#define ASSERT_REGISTERS_16BIT_EQ(expected)                    \
    do {                                                       \
        ASSERT_EQ((expected)[0], read_register_16bit(REG_AF)); \
        ASSERT_EQ((expected)[1], read_register_16bit(REG_BC)); \
        ASSERT_EQ((expected)[2], read_register_16bit(REG_DE)); \
        ASSERT_EQ((expected)[3], read_register_16bit(REG_HL)); \
    } while (0);

#undef NB_REG
#define NB_REG 4

// Empty registers (should read 0)
TEST_F(CPURegisters16bit, NullValues)
{
    u16 test[NB_REG]{0, 0, 0, 0};

    ASSERT_REGISTERS_16BIT_EQ(test);
}

// Registers with all bits set
TEST_F(CPURegisters16bit, MaxValue)
{
    u16 expected[NB_REG] = {0};
    memset(expected, 0xFFFF, sizeof(expected));

    SET_16BIT_REG(0xFFFF);

    ASSERT_REGISTERS_16BIT_EQ(expected);
}

TEST_P(CPURegisters16bit, Read)
{
    const auto &val = GetParam();
    u16 expected[NB_REG] = {val, val, val, val};

    const u8 lsb = LSB(val);
    const u8 msb = MSB(val);

    g_cpu.registers = {msb, lsb, msb, lsb, msb, lsb, msb, lsb};

    ASSERT_REGISTERS_16BIT_EQ(expected);
}

TEST_P(CPURegisters16bit, Write)
{
    SET_16BIT_REG(GetParam());

    const auto &val = GetParam();
    const auto &reg = g_cpu.registers;

    const auto reg_values = {(reg.a << 8) | reg.f, (reg.b << 8) | reg.c,
                             (reg.d << 8) | reg.e, (reg.h << 8) | reg.l};

    ASSERT_TRUE(std::all_of(reg_values.begin(), reg_values.end(),
                            [&val](u16 x) { return x == val; }));
}

// Make sure we don't accidentally write onto other registers
TEST_F(CPURegisters16bit, WriteCorrectRegister)
{
    MAP_16BIT_REG([](int reg) {
        SET_16BIT_REG(0);
        write_register_16bit(static_cast<cpu_register_name>(reg), 0xFFFF);
        MAP_16BIT_REG([&reg](int i) {
            auto cast = static_cast<cpu_register_name>(i);
            ASSERT_EQ(read_register_16bit(cast), (i == reg) ? 0xFFFF : 0);
        });
    });
}

// Read native 16-bit registers (PC/SP)
TEST_F(CPURegisters16bit, ReadSpecialRegister)
{
    const u16 val = 0x0727;

    g_cpu.registers.pc = val;
    g_cpu.registers.sp = val | 0x1000;

    auto pc = read_register_16bit(REG_PC);
    auto sp = read_register_16bit(REG_SP);

    ASSERT_EQ(pc, val);
    ASSERT_EQ(sp, val | 0x1000);
}

// Write native 16-bit registers (PC/SP)
TEST_F(CPURegisters16bit, WriteSpecialRegister)
{
    const u16 val = 0x0727;

    write_register_16bit(REG_PC, val);
    write_register_16bit(REG_SP, val | 0x1000);

    ASSERT_EQ(g_cpu.registers.pc, val);
    ASSERT_EQ(g_cpu.registers.sp, val | 0x1000);
}

// Make sure the registers are initialised correctly
TEST_F(CPURegisters16bit, InitialRegisterValues)
{
    u16 expected[NB_REG] = {0x01B0, 0x0013, 0x00D8, 0x014D};

    reset_cpu();

    ASSERT_REGISTERS_16BIT_EQ(expected);
    ASSERT_EQ(g_cpu.registers.pc, 0x0100);
    ASSERT_EQ(g_cpu.registers.sp, 0xFFFE);
}

// Test cases:
// - null value (0x0000)
// - max value (0xFFFF)
// - One byte set only (0xFF00 ; 0x00FF ; ...)
// - "mirror" values (0x8001; 0x0180; ...)
INSTANTIATE_TEST_SUITE_P(CPURegisters, CPURegisters16bit,
                         ::testing::Values(0x00, 0xFF, 0x80, 0x01, 0xF0, 0x0F,
                                           0x18, 0x81, 0x5A));

#pragma endregion tests_16bit

} // namespace cpu_tests
