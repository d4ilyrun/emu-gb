#define REG_ERR REG_ERR_
#include <gtest/gtest-death-test.h>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <cpu/cpu.h>
#include <cpu/memory.h>
#include <cartridge/cartridge.h>
#include <cartridge/memory.h>
}

#include "cartridge.hxx"

namespace cartridge_tests
{

template <uint rom>
class MBC2Generator : public CartridgeGenerator<rom, 512>,
                      public ::testing::Test
{
  public:
    MBC2Generator() : CartridgeGenerator<rom, 512>(MBC2) {}

    void SetUp()
    {
        // Manually set the newly generated cartridge as loaded
        cartridge = this->cart_;
    };

  protected:
    inline void enable_ram()
    {
        chip_registers.ram_g = 0xA;
        ram_access = false;
    }
};

using MBC2_Registers = MBC2Generator<1 << 18>;

TEST_F(MBC2_Registers, RAMEnable)
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

    // Write anything else than 0xA with BIT 8 clear: deactivate
    write_mbc2(0x12FF, 0xB);
    ASSERT_FALSE(ram_access);

    // Write 0xA with BIT 8 set: do not change
    const auto tmp = chip_registers.rom_bank;
    write_mbc2(0x3FF, 0xA);
    ASSERT_FALSE(ram_access);
    chip_registers.rom_bank = tmp;
}

TEST_F(MBC2_Registers, ROMBankNumber)
{
    // When BIT 8 is set: control ROM
    // Set to 1 by default
    ASSERT_EQ(chip_registers.rom_bank, 1);

    // The lower 4 bits of the address control the rom bank number
    write_mbc2(0x10F, 0xF);
    ASSERT_EQ(chip_registers.rom_bank, 0xF);

    write_mbc2(0x10A, 0x3A);
    ASSERT_EQ(chip_registers.rom_bank, 0xA);

    // Similar to MBC1, can never be null and should be replaced by 1
    write_mbc2(0x100, 0x0);
    ASSERT_EQ(chip_registers.rom_bank, 0x1);

    // Value shouldn't change when the address' 8th BIT is clear
    chip_registers.rom_bank = 0xA;
    write_mbc2(0x20A, 0xF);
    ASSERT_EQ(chip_registers.rom_bank, 0xA);
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

        chip_registers.rom_bank = param.rom_bank;
        if (param.ram_access)
            this->enable_ram();

        // Reset ROM
        memset(this->cart_.rom, 0, this->cart_.rom_size);

        cartridge = this->cart_;
    }
};

using MBC2 = MBC2RWGenerator<1 << 18>;

TEST_P(MBC2, Read)
{
    const auto &param = GetParam();
    auto value = param.value;

    if (param.address >= 0xA000 && param.address < 0xA200) {
        value |= 0xF;
        cartridge.ram[param.expected] = value;
        if (!ram_access)
            value = 0xFF;
    } else {
        cartridge.rom[param.expected] = value;
    }

    ASSERT_EQ(read_mbc2(param.address), value);
}

TEST_P(MBC2, Write)
{
    const auto &param = GetParam();

    write_mbc2(param.address, param.value);

    if (param.address >= 0xA000 && param.address < 0xA200) {
        if (ram_access)
            ASSERT_EQ(cartridge.ram[param.expected], param.value | 0xF);
        else
            ASSERT_EQ(cartridge.ram[param.expected], 0);
    } else {
        ASSERT_EQ(cartridge.rom[param.expected], 0);
    }
}

INSTANTIATE_TEST_SUITE_P(
    Memory_8bit, MBC2,
    ::testing::Values(
        // ROM
        (struct mbc2_rw_param){0x12A7, 0x42, false, 0x4, 0x12A7},
        (struct mbc2_rw_param){0x54C1, 0x42, false, 0x4, 0x114C1},
        (struct mbc2_rw_param){0x0000, 0x42, true, 0x4, 0x0000},
        (struct mbc2_rw_param){0x7FFF, 0x42, true, 0xF, 0x3FFFF}, // Max value
        (struct mbc2_rw_param){0x4000, 0x42, true, 0x0, 0x4000},
        // RAM
        (struct mbc2_rw_param){0xA1FF, 0x42, true, 0x4, 0x1FF},
        (struct mbc2_rw_param){0xA01B, 0x42, true, 0x4, 0x01B},
        (struct mbc2_rw_param){0xA01B, 0x42, false, 0x4, 0x01B}));

// 32KiB ROM
using MBC2_Death = MBC2RWGenerator<1 << 15>;

// No tests for writing because we either write into a predtermined address
// range (no computation for the RAM) or into the ROM.
// Thus, it is impossible to write outside of the boundaries.

TEST_P(MBC2_Death, Read)
{
    const auto &param = GetParam();

    if (param.address < 0x8000)
        ASSERT_GT(param.expected, cartridge.rom_size);
    else
        GTEST_SKIP() << "Cannot read outside of RAM.";

    EXPECT_DEATH(read_mbc2(param.address), "");
}

INSTANTIATE_TEST_SUITE_P(Memory_8bit, MBC2_Death,
                         ::testing::Values((struct mbc2_rw_param){
                             0x7FFF, 0x42, true, 0x10, 0x3FFFF}));
} // namespace cartridge_tests
