// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include "CPU/cpu.h"
}

namespace tests
{

class TestCPURegisters : public ::testing::Test
{
  public:
    TestCPURegisters() {}

    void SetUp()
    {
        memset(&cpu.registers, 0, sizeof(struct cpu_registers));
    }
};

#pragma region tests_8bit

using CPURegisters8bit = TestCPURegisters;

/**
 * \brief Map a function to each of the CPU's 8bits registers
 */
#define MAP_8BIT_REG(foo)                      \
    do {                                       \
        for (int i = REG_A; i <= REG_L; ++i) { \
            foo(i);                            \
        }                                      \
    } while (0);

#define SET_8BIT_REG(value) \
    MAP_8BIT_REG([](int i) { write_register((cpu_register_name)i, value); })

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

// Registers with all bits set
TEST_F(CPURegisters8bit, MaxValue)
{
    u8 expected[NB_REG] = {0};
    memset(expected, 0xFF, sizeof(expected));

    SET_8BIT_REG(0xFF);

    ASSERT_REGISTERS_EQ(expected);
}

// MSB is set everytime (0x80)
TEST_F(CPURegisters8bit, MSB)
{
    u8 expected[NB_REG] = {0};
    memset(expected, 0x80, sizeof(expected));

    SET_8BIT_REG(0x80);

    ASSERT_REGISTERS_EQ(expected);
}

// MSB is set everytime (0x01)
TEST_F(CPURegisters8bit, LSB)
{
    u8 expected[NB_REG] = {0};
    memset(expected, 0x01, sizeof(expected));

    SET_8BIT_REG(0x01);

    ASSERT_REGISTERS_EQ(expected);
}

#pragma endregion tests_8bit

#pragma region tests_16bit

using CPURegisters16bit = TestCPURegisters;

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

// Make sure the 16bit registers are read in correct order (bits 9-16 set)
TEST_F(CPURegisters16bit, ReadByteOrderMSB)
{
    u16 expected[NB_REG] = {0};

    for (auto i = 0; i < NB_REG; ++i)
        expected[i] = 0xFF00;

    cpu.registers.a = 0xFF;
    cpu.registers.b = 0xFF;
    cpu.registers.d = 0xFF;
    cpu.registers.h = 0xFF;

    ASSERT_REGISTERS_16BIT_EQ(expected);
}

// Make sure the 16bit registers are read in correct order (bits 1-8 set)
TEST_F(CPURegisters16bit, ReadByteOrderLSB)
{
    u16 expected[NB_REG] = {0};

    for (auto i = 0; i < NB_REG; ++i)
        expected[i] = 0x00FF;

    cpu.registers.f = 0xFF;
    cpu.registers.c = 0xFF;
    cpu.registers.e = 0xFF;
    cpu.registers.l = 0xFF;

    ASSERT_REGISTERS_16BIT_EQ(expected);
}

// Make sure the 16bit registers are correcly reordered when read
TEST_F(CPURegisters16bit, ReadByteOrder)
{
    u16 expected[NB_REG] = {0};

    for (auto i = 0; i < NB_REG; ++i)
        expected[i] = 0xAABB;

    cpu.registers = {0xAA, 0xBB, 0xAA, 0xBB, 0xAA, 0xBB, 0xAA, 0xBB};

    ASSERT_REGISTERS_16BIT_EQ(expected);
}

// Make sure the 16bit registers are written in the correct order (bits 9-16
// set)
TEST_F(CPURegisters16bit, WriteByteOrderMSB)
{
    write_register_16bit(REG_AF, 0xFF00);
    ASSERT_EQ(cpu.registers.a, 0xFF);
    ASSERT_EQ(cpu.registers.f, 0x00);
}

// Make sure the 16bit registers are written in the correct order (bits 1-8 set)
TEST_F(CPURegisters16bit, WriteByteOrderLSB)
{
    write_register_16bit(REG_AF, 0x00FF);
    ASSERT_EQ(cpu.registers.a, 0x00);
    ASSERT_EQ(cpu.registers.f, 0xFF);
}

// Make sure the 16bit registers are written in the correct order
TEST_F(CPURegisters16bit, WriteByteOrder)
{
    write_register_16bit(REG_AF, 0xAABB);
    ASSERT_EQ(cpu.registers.a, 0xAA);
    ASSERT_EQ(cpu.registers.f, 0xBB);
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

#pragma endregion tests_16bit

} // namespace tests
