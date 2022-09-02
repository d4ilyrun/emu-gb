#define REG_ERR REG_ERR_
#include <gtest/gtest-death-test.h>
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <cpu/memory.h>
#include <cartridge/cartridge.h>
#include <cartridge/memory.h>
}

#include "cartridge.hxx"

namespace cartridge_tests
{

class ROMOnly : public CartridgeGenerator<1 << 15>,
                public ::testing::TestWithParam<u16>
{
  public:
    ROMOnly() : CartridgeGenerator(MBC1) {}

    void SetUp()
    {
        // Manually set the newly generated cartridge as loaded
        cartridge = this->cart_;
    };
};

TEST_P(ROMOnly, Write)
{
    const auto &address = GetParam();
    const auto value = 0x1F;

    write_cartridge(address, value);

    ASSERT_EQ(cartridge.rom[address], value);
}

TEST_P(ROMOnly, Read)
{
    const auto &address = GetParam();
    const auto value = 0x1F;

    cartridge.rom[address] = value;

    ASSERT_EQ(read_cartridge(address), value);
}

INSTANTIATE_TEST_SUITE_P(Memory_8bit, ROMOnly,
                         ::testing::Values(0, 0x7FFF, 0x001F));

using ROMOnly_Death = ROMOnly;

TEST_F(ROMOnly_Death, OutOfRange)
{
    ASSERT_DEATH(write_cartridge(0x8000, 0x69), "");
    ASSERT_DEATH(read_cartridge(0x8000), "");
};

}; // namespace cartridge_tests
