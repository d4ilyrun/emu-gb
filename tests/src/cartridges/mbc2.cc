#define REG_ERR REG_ERR_
#include <gtest/gtest-death-test.h>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <CPU/cpu.h>
#include <CPU/memory.h>
#include <cartridge/cartridge.h>
#include <cartridge/memory.h>
}

#include "cartridge.hxx"

namespace cartridge_tests
{

template <uint rom>
class MBC2Generator : public CartridgeGenerator<rom, 0x200>,
                      public ::testing::Test
{
  public:
    MBC2Generator() : CartridgeGenerator<rom, 0x200>(MBC2) {}

    void SetUp()
    {
        // Manually set the newly generated cartridge as loaded
        this->cart_.ram = &cpu.memory[0xA000];
        cartridge = this->cart_;
    };

  protected:
    inline void enable_ram()
    {
        chip_registers.ram_g = 0xA;
        ram_access = false;
    }
};

using MBC2Registers = MBC2Generator<1 << 18>;

TEST_F(MBC2Registers, RAMEnable)
{
    // When BIT 8 is clear: control RAM
    // RAM is disabled by default
    ASSERT_FALSE(ram_access);
    ASSERT_NE(chip_registers.ram_g, 0xA);

    // Write 0xA with BIT 8 clear: activate ram access
    write_mbc2(0x200, 0xA);
    ASSERT_TRUE(ram_access);

    write_mbc2(0xFF, 0x1A);
    ASSERT_TRUE(ram_access);

    // Write 0xA with BIT 8 set: deactivate
    write_mbc2(0x3FF, 0xA);
    ASSERT_FALSE(ram_access);

    // Write anything else than 0xA with BIT 8 clear: deactivate
    write_mbc2(0x12FF, 0xB);
    ASSERT_FALSE(ram_access);
}

TEST_F(MBC2Registers, ROMBankNumber)
{
    // When BIT 8 is set: control ROM
    // Set to 1 by default
    ASSERT_EQ(chip_registers.rom_b, 1);

    // The lower 4 bits of the address control the rom bank number
    write_mbc2(0x10F, 0x0);
    ASSERT_EQ(chip_registers.rom_b, 0xF);

    write_mbc2(0x10A, 0x0);
    ASSERT_EQ(chip_registers.rom_b, 0xA);

    // Similar to MBC1, can never be null and should be replaced by 1
    write_mbc2(0x100, 0x0);
    ASSERT_EQ(chip_registers.rom_b, 0x1);

    // Value shouldn't change when the address' 8th BIT is clear
    chip_registers.rom_b = 0xA;
    write_mbc2(0x20A, 0xF);
    ASSERT_EQ(chip_registers.rom_b, 0xA);
}

struct mbc2_rw_param {
    u16 address;
    u8 value;

    // registers
    bool ram_access;
    u8 rom_bank;

    // Expected resulting address
    u32 expected;
};

template <uint rom>
class MBC2RWGenerator : public MBC2Generator<rom>,
                        public ::testing::WithParamInterface<mbc2_rw_param>
{
    void SetUp() override
    {
        const auto &param = GetParam();

        chip_registers.rom_b = param.rom_bank;
        if (param.ram_access)
            this->enable_ram();

        // Reset ROM
        memset(this->cart_.rom, 0, this->cart_.rom_size);

        cartridge = this->cart_;
    }
};

using MBC2_read_write = MBC2RWGenerator<1 << 18>;

TEST_P(MBC2_read_write, Read)
{
    const auto &param = GetParam();
    auto value = param.value;

    if (param.address >= 0xA000 && param.address < 0xA200) {
        value |= 0xF;
        cartridge.ram[param.address & 0x1FF] = value;
        if (!ram_access)
            value = 0xFF;
    } else {
        cartridge.rom[param.expected] = value;
    }

    ASSERT_EQ(read_mbc2(param.address), value);
}

TEST_P(MBC2_read_write, Write)
{
    const auto &param = GetParam();

    write_mbc2(param.address, param.value);

    if (param.address >= 0xA000 && param.address < 0xA200) {
        if (ram_access)
            ASSERT_EQ(cartridge.ram[param.address & 0x1FF], param.value | 0xF);
        else
            ASSERT_EQ(cartridge.ram[param.address & 0x1FF], 0);
    } else {
        ASSERT_EQ(cartridge.rom[param.expected], param.value);
    }
}

INSTANTIATE_TEST_SUITE_P(
    Memory, MBC2_read_write,
    ::testing::Values(
        // ROM
        (struct mbc2_rw_param){0x12A7, 0x42, false, 0x4, 0x12A7},
        (struct mbc2_rw_param){0x54C1, 0x42, false, 0x4, 0x94C1},
        (struct mbc2_rw_param){0x0000, 0x42, true, 0x4, 0x0000},
        (struct mbc2_rw_param){0x7FFF, 0x42, true, 0xF, 0x3FFFF}, // Max value
        // RAM
        (struct mbc2_rw_param){0xA1FF, 0x42, true, 0x4, 0x0000},
        (struct mbc2_rw_param){0xA01B, 0x42, true, 0x4, 0x0000},
        (struct mbc2_rw_param){0xA01B, 0x42, false, 0x4, 0x0000},
        (struct mbc2_rw_param){0xA000, 0x77, true, 0x0, 0x5123}));

// 32KiB ROM
using MBC2_out_of_range = MBC2RWGenerator<1 << 15>;

// No tests for writing because we either write into a predtermined address
// range (no computation for the RAM) or into the ROM.
// Thus, it is impossible to write outside of the boundaries.

TEST_P(MBC2_out_of_range, Read)
{
    const auto &param = GetParam();

    if (param.address < 0x8000)
        EXPECT_GT(param.expected, sizeof(cartridge.rom));
    else
        GTEST_SKIP() << "Cannot read outside of RAM.";

    EXPECT_DEATH(write_mbc1(param.address, param.value), "");
}

INSTANTIATE_TEST_SUITE_P(Memory, MBC2_out_of_range,
                         ::testing::Values((struct mbc2_rw_param){
                             0x7FFF, 0x42, true, 0xF, 0x3FFFF}));
} // namespace cartridge_tests
