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

template <uint size, typename param_type>
class LoadRelativeTest : public LoadInstructionTest<size, param_type>
{
  public:
    void TearDown() override
    {
        InstructionTest::TearDown();
        const auto &param = this->GetParam();
        ASSERT_EQ(read_memory(param.address), param.expected);
    }
};

/*
 * Relative addressing includes:
 * - From the address in 16bit reg -> 8bit register
 * - From an 8bit reg -> the address in a 16bit register
 * - Immediate value -> the address in the HL register
 * - From an immediate address -> A register
 * - From the A register -> an immediate address
 * - From SP -> relative immediate address
 * - A <- (FF00+n)
 * - (FF00+n) <- A
 * - A <- (FF00+C)
 * - (FF00+C) <- A
 */

// Value stored at the address stored in a 16bit reg -> 8bit register
struct adr_to_reg {
    u8 instruction[1];
    cpu_register_name dst;
    cpu_register_name reg;
    u16 address;
    u8 expected = 0x42;
    u8 *out = nullptr; // Has to be here but not used
};

using AddressInRegisterToRegister = LoadRelativeTest<1, adr_to_reg>;

// Value from an 8bit reg -> the address in a 16bit register
struct reg_to_adr {
    u8 instruction[1];
    cpu_register_name reg_out; // 16bit
    cpu_register_name reg_in;  // 8bit
    u16 address;
    u8 expected = 0x42;
    u8 *out = nullptr;
};

using RegisterToAddressInRegister = LoadRelativeTest<1, reg_to_adr>;

// Immediate value -> the address in the HL register
struct ldi_params {
    u16 address;
    u8 instruction[2] = {0x36, 0x42};
    u8 expected = 0x42;
    u8 *out = nullptr;
};

using ImmediateToRelativeHL = LoadRelativeTest<2, ldi_params>;

// From an immediate address -> A register
struct imm_adr_to_a {
    u8 instruction[3];
    u16 address;
    u8 expected = 0x42;
    u8 *out = nullptr;
};

using ImmediateAddressToA = LoadRelativeTest<3, imm_adr_to_a>;

// From the A register -> an immediate address
using AToImmediateAddress =
    LoadRelativeTest<3, imm_adr_to_a>; // Same basically !
                                       //
TEST_P(AddressInRegisterToRegister, Load)
{
    const auto &param = GetParam();
    write_register_16bit(param.reg, param.address);
    write_memory(param.address, param.expected);
    execute_instruction();
    ASSERT_EQ(read_register(param.dst), param.expected);
}

TEST_P(RegisterToAddressInRegister, Load)
{
    const auto &param = GetParam();
    write_register_16bit(param.reg_out, param.address);
    write_register(param.reg_in, param.expected);
    execute_instruction();
}

TEST_P(ImmediateToRelativeHL, Load)
{
    write_register_16bit(REG_HL, GetParam().address);
    execute_instruction();
}

TEST_P(ImmediateAddressToA, Load)
{
    const auto &param = GetParam();
    const auto address = (param.instruction[2] << 8) | param.instruction[1];
    EXPECT_EQ(address, param.address);
    cpu.registers.a = param.expected - 1;
    write_memory(address, param.expected);
    execute_instruction();
    ASSERT_EQ(cpu.registers.a, param.expected);
}

TEST_P(AToImmediateAddress, Load)
{
    const auto &param = GetParam();
    const auto address = (param.instruction[2] << 8) | param.instruction[1];
    EXPECT_EQ(address, param.address);
    cpu.registers.a = param.expected;
    execute_instruction();
}

const std::vector<adr_to_reg> address_to_register{
    {{0b01000110}, REG_B, REG_HL, 0x0},    // LD B, (HL)
    {{0b01001110}, REG_C, REG_HL, 0x7FFF}, // LD C, (HL)
    {{0b00001010}, REG_A, REG_BC, 0x0000}, // LD A, (BC)
    {{0b00011010}, REG_A, REG_DE, 0x1234}, // LD A, (DE)
};

INSTANTIATE_TEST_SUITE_P(LoadRelative, AddressInRegisterToRegister,
                         ::testing::ValuesIn(address_to_register));

const std::vector<reg_to_adr> register_to_address{
    {0x70, REG_HL, REG_B, 0xFF},         // LD (HL), B
    {0x71, REG_HL, REG_C, 0x7FFF},       // LD (HL), C
    {0x72, REG_HL, REG_D, 0x0000},       // LD (HL), D
    {0x73, REG_HL, REG_E, 0x1234},       // LD (HL), E
    {0x74, REG_HL, REG_H, 0x4321, 0x43}, // LD (HL), H
    {0x75, REG_HL, REG_L, 0x1FF1, 0xF1}, // LD (HL), L
    {0x77, REG_HL, REG_A, 0x0F0F},       // LD (HL), A
    {0x12, REG_DE, REG_A, 0x0F0F},       // LD (DE), A
    {0x02, REG_BC, REG_A, 0x0F0F},       // LD (BC), A
};

INSTANTIATE_TEST_SUITE_P(LoadRelative, RegisterToAddressInRegister,
                         ::testing::ValuesIn(register_to_address));

// Start of some memory areas inside a cartridge
const std::vector<ldi_params> immediate_to_hl{{0xFFFF}, {0x0000}, {0x4000},
                                              {0x8000}, {0xA000}, {0xC000},
                                              {0xD000}, {0xE000}};

INSTANTIATE_TEST_SUITE_P(LoadRelative, ImmediateToRelativeHL,
                         ::testing::ValuesIn(immediate_to_hl));

#define CASE(msb_, lsb_) {0xFA, lsb_, msb_}, (msb_ << 8) | lsb_
const std::vector<imm_adr_to_a> immediate_adr_to_a{
    {CASE(0xFF, 0xFF)}, {CASE(0x00, 0x00)}, {CASE(0x40, 0x00)},
    {CASE(0x80, 0x00)}, {CASE(0xA0, 0x00)}, {CASE(0xC0, 0x00)},
    {CASE(0xD0, 0x00)}, {CASE(0xE0, 0x00)},
};
#undef CASE

INSTANTIATE_TEST_SUITE_P(LoadRelative, ImmediateAddressToA,
                         ::testing::ValuesIn(immediate_adr_to_a));

#define CASE(msb_, lsb_) {0xEA, lsb_, msb_}, (msb_ << 8) | lsb_
const std::vector<imm_adr_to_a> a_to_immediate_adr{
    {CASE(0xFF, 0xFF)}, {CASE(0x00, 0x00)}, {CASE(0x40, 0x00)},
    {CASE(0x80, 0x00)}, {CASE(0xA0, 0x00)}, {CASE(0xC0, 0x00)},
    {CASE(0xD0, 0x00)}, {CASE(0xE0, 0x00)},
};
#undef CASE

INSTANTIATE_TEST_SUITE_P(LoadRelative, AToImmediateAddress,
                         ::testing::ValuesIn(a_to_immediate_adr));

// NON PARAMETERIZED TESTS

class SPToImmediateAddress : public InstructionTest
{
  public:
    SPToImmediateAddress() : InstructionTest(3) {}
};

class RelativeToFF00 : public InstructionTest
{
  public:
    RelativeToFF00() : InstructionTest(2) {}
};

TEST_F(SPToImmediateAddress, LoadRelative)
{
    const u16 address = 0x1234;
    const u8 instruction[3] = {0x08, LSB(address), MSB(address)};

    cpu.registers.sp = address;
    Load((void *)instruction);
    execute_instruction();

    ASSERT_EQ(read_memory_16bit(address), cpu.registers.sp);
}

TEST_F(RelativeToFF00, OffsetFromC)
{
    u8 instruction[2] = {0xF2, 0x00};
    const u8 offset = 0x42;
    const u8 data = 0x69;

    Load((void *)instruction);

    write_memory(0xFF00 | offset, data);
    cpu.registers.c = offset;
    execute_instruction();
    ASSERT_EQ(cpu.registers.a, data);

    instruction[0] = 0xE2;
    Load((void *)instruction);

    write_memory(0xFF00 | offset, 0);
    execute_instruction();
    ASSERT_EQ(read_memory(0xFF00 | offset), cpu.registers.a);
}

TEST_F(RelativeToFF00, OffsetFromImmediateValue)
{
    const u8 offset = 0x42;
    const u8 data = 0x69;
    u8 instruction[2] = {0xF0, offset};

    Load((void *)instruction);

    write_memory(0xFF00 | offset, data);
    cpu.registers.c = offset;
    execute_instruction();
    ASSERT_EQ(cpu.registers.a, data);

    cpu.registers.pc -= 2; // Necessary for TearDown's test
    instruction[0] = 0xE0;
    Load((void *)instruction);

    write_memory(0xFF00 | offset, 0);
    execute_instruction();
    ASSERT_EQ(read_memory(0xFF00 | offset), cpu.registers.a);
}

}; // namespace cpu_tests
