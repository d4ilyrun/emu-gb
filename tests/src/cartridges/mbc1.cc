#define REG_ERR REG_ERR_
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <CPU/memory.h>
#include <cartridge/cartridge.h>
#include <cartridge/memory.h>
}

#include "cartridge.hxx"

namespace cartridge_tests
{

template <uint rom, uint ram>
class MBC1Generator : public CartridgeGenerator<rom, ram>,
                      public ::testing::Test
{
  public:
    MBC1Generator() : CartridgeGenerator<rom, ram>(MBC1) {}

    void SetUp() override
    {
        // Manually set the newly generated cartridge as loaded
        cartridge = this->GetCart();
    };

    void TearDown() override {}

  private:
    inline void enable_ram()
    {
        chip_registers.ram_g = 0xA;
    }
};

// 2 MiB ROM & 32 KiB RAM
class MBC1_Registers : public MBC1Generator<1 << 21, 1 << 16>
{
};

// Update RAM access when writing to 0x0000 -> 0x1FFF
TEST_F(MBC1_Registers, EnableRAM)
{
    const auto address = 0x0010; // Between 0x0000 and 0x1FFF

    EXPECT_EQ(chip_registers.ram_g, 0x0);
    ASSERT_FALSE(ram_access);

    // Write to ram register
    // First bits should be ignored, 1A -> A
    write_mbc1(address, 0x1A);
    ASSERT_TRUE(ram_access);
    ASSERT_EQ(chip_registers.ram_g, 0xA);

    write_mbc1(address, 0x1B);
    ASSERT_FALSE(ram_access);
    ASSERT_EQ(chip_registers.ram_g, 0xB);

    // Write outside of the designated area, shouldn't modify its content
    write_mbc1(0x2000, 0x1A);
    ASSERT_EQ(chip_registers.ram_g, 0xB);
    ASSERT_FALSE(ram_access);
}

// Update the lower 5 bits of the ROM bank number
// when writing to 0x2000 -> 0x3FFF
TEST_F(MBC1_Registers, ROMBankNumber)
{
    const auto address = 0x3113;

    // The lower bits can never be set to 0 and should be set to 1 instead
    ASSERT_TRUE(chip_registers.bank_1 != 0);

    chip_registers.bank_1 = 1;

    write_mbc1(address, 0x03);
    ASSERT_EQ(chip_registers.bank_1, 0x03);

    // Higher bits are discarded
    write_mbc1(address, 0xE1);
    ASSERT_EQ(chip_registers.bank_1, 0x01);

    // When writing 0, automatically change it to one
    write_mbc1(address, 0);
    ASSERT_EQ(chip_registers.bank_1, 1);
}

// If the ROM Bank Number is set to a higher value than the number of banks in
// the cart, the bank number is masked to the required number of bits.
TEST_F(MBC1_Registers, ROMBankNumberMasking)
{
    // TODO
    GTEST_SKIP_("TODO: Not implemented");
}

// Select RAM bank number (from 0x00 to 0x03) when writing to 0x4000 - 0x5FFF
// Specify the upper two bibts of the ROM bank number (1MiB ROM or larger)
TEST_F(MBC1_Registers, RAMBankNumber)
{
    const auto address = 0x5025;

    ASSERT_EQ(chip_registers.bank_2, 0);

    write_mbc1(address, 0b0101);
    ASSERT_EQ(chip_registers.bank_2, 1);

    write_mbc1(address, 0b0011);
    ASSERT_EQ(chip_registers.bank_2, 3);

    write_mbc1(address, 0xFC);
    ASSERT_EQ(chip_registers.bank_2, 0);
}

// When writing to 0x6000 - 0x7FFF, switch between RAM and ROM banking mode
TEST_F(MBC1_Registers, BankingModeSelect)
{
    const auto address = 0x6025;

    // default value
    ASSERT_EQ(chip_registers.mode, 0);

    write_mbc1(address, 0);
    ASSERT_EQ(chip_registers.mode, 0);

    write_mbc1(address, 0x15);
    ASSERT_EQ(chip_registers.mode, 1);

    write_mbc1(address, 0x14);
    ASSERT_EQ(chip_registers.mode, 0);
}

struct mbc1_rw_param {
    u16 address;
    u16 value;

    // registers
    bool ram_access;
    u8 rom_bank;
    u8 ram_bank;
    u8 mode;

    // Expected resulting address
    u32 expected;
};

template <uint rom, uint ram>
class MBC1RWGenerator : public MBC1Generator<rom, ram>,
                        public ::testing::WithParamInterface<mbc1_rw_param>
{
    void SetUp() override
    {
        const auto &param = GetParam();

        ram_access = param.ram_access;
        chip_registers.bank_1 = param.rom_bank;
        chip_registers.bank_2 = param.ram_bank;
        chip_registers.mode = param.mode;

        if (ram_access)
            chip_registers.ram_g = 0xA;

        // Reset ROM
        memset(this->cart_.rom, 0, this->cart_.rom_size);

        cartridge = this->cart_;
    }
};

using MBC1 = MBC1RWGenerator<1 << 21, 1 << 15>;

TEST_P(MBC1, Write)
{
    const auto &param = GetParam();

    write_mbc1(param.address, param.value);

    if (param.address < 0xA000)
        ASSERT_EQ(cartridge.rom[param.expected], 0);
    else {
        if (param.ram_access)
            ASSERT_EQ(cartridge.ram[param.expected], param.value);
        else
            ASSERT_EQ(cartridge.ram[param.expected], 0);
    }
}

TEST_P(MBC1, Read)
{
    const auto &param = GetParam();
    auto expected = param.value;

    if (param.address < 0xA000) {
        cartridge.rom[param.expected] = param.value;
    } else {
        cartridge.ram[param.expected] = param.value;
        if (!param.ram_access)
            expected = 0xFF;
    }

    const auto read = read_mbc1(param.address);
    ASSERT_EQ(read, expected);
}

INSTANTIATE_TEST_SUITE_P(
    Memory_8bit, MBC1,
    ::testing::Values(
        // ROM
        (struct mbc1_rw_param){0x21B3, 0x69, false, 0x12, 0b01, 0, 0x21B3},
        (struct mbc1_rw_param){0x21B3, 0x69, false, 0x12, 0b01, 1, 0x821B3},
        (struct mbc1_rw_param){0x72A7, 0x42, false, 0x04, 0b10, 1, 0x1132A7},
        // RAM
        (struct mbc1_rw_param){0xB123, 0x77, true, 0x12, 0b10, 0, 0x1123},
        (struct mbc1_rw_param){0xB123, 0x77, true, 0x12, 0b10, 1, 0x5123},
        (struct mbc1_rw_param){0xB123, 0x77, false, 0x12, 0b10, 1, 0x5123},
        (struct mbc1_rw_param){0xB123, 0x77, true, 0x00, 0b10, 1, 0x5123}));

// 64 KiB ROM & 8 KiB RAM
using MBC1_Death = MBC1RWGenerator<1 << 16, 1 << 13>;

TEST_P(MBC1_Death, Write)
{
    const auto &param = GetParam();

    if (param.address < 0xA000)
        GTEST_SKIP() << "Writing into ROM";
    else
        EXPECT_GT(param.expected, sizeof(cartridge.ram));

    EXPECT_DEATH(write_mbc1(param.address, param.value), "");
}

TEST_P(MBC1_Death, Read)
{
    const auto &param = GetParam();

    if (param.address < 0xA000)
        EXPECT_GT(param.expected, sizeof(cartridge.rom));
    else
        EXPECT_GT(param.expected, sizeof(cartridge.ram));

    EXPECT_DEATH(read_mbc1(param.address), "");
}

INSTANTIATE_TEST_SUITE_P(
    Memory_8bit, MBC1_Death,
    ::testing::Values(
        // ROM
        (struct mbc1_rw_param){0x72A7, 0x42, false, 0x04, 0b10, 1, 0x1132A7},
        // RAM
        (struct mbc1_rw_param){0xB123, 0x77, true, 0x00, 0b10, 1, 0x5123}));

// TODO: Tests for multibanking

} // namespace cartridge_tests
