// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include "cpu/memory.h"
#include "options.h"
#include "ppu/lcd.h"
#include "ppu/ppu.h"
}

namespace ppu_test
{

class PPUTest : public ::testing::Test
{
  public:
    PPUTest() : ppu_{ppu_get()} {}

    void SetUp() override
    {
        get_options()->log_level = LOG_WARNING;
        ppu_init();
    }

    void TearDown() override
    {
        free(ppu_->tile_data);
    }

  protected:
    const struct ppu *ppu_;
};

#ifdef OAM
#undef OAM
#endif

using PPU = PPUTest;
using VRAM = PPUTest;
using OAM = PPUTest;

TEST_F(PPU, Default)
{
    ASSERT_NE(ppu_->tile_data, nullptr);
    ASSERT_EQ(lcd_get_mode(), MODE_OAM);
}

TEST_F(VRAM, Simple)
{
    write_vram(0x8000, 42);
    ASSERT_EQ(ppu_->tile_data[0x00], 42);
    ASSERT_EQ(read_vram(0x8000), 42);

    write_vram(0x9845, 42);
    ASSERT_EQ(ppu_->tile_maps[0][0x45], 42);
    ASSERT_EQ(read_vram(0x9845), 42);

    write_vram(0x9FFF, 42);
    ASSERT_EQ(ppu_->tile_maps[1][0x3FF], 42);
    ASSERT_EQ(read_vram(0x9FFF), 42);

    ASSERT_DEATH(write_vram(0x7999, 42), ".*");
    ASSERT_DEATH(write_vram(0xA000, 42), ".*");
}

TEST_F(OAM, Simple)
{
    const u8 *oam = (const u8 *)ppu_->oam;

    write_oam(0xFE45, 42);
    ASSERT_EQ(oam[0x45], 42);
    ASSERT_EQ(read_oam(0xFE45), 42);

    ASSERT_DEATH(write_oam(0xFEA0, 42), ".*");
}

/*
 * When the PPU is accessing some video-related memory, that
 * memory is inaccessible to the CPU: writes are ignored, and
 * reads return garbage values (usually $FF).
 */

TEST_F(VRAM, CPU_Mode3)
{
    // During mode 3, the CPU cannot access VRAM or CGB
    // palette data registers ($FF69,$FF6B).

    // Set mode 3
    lcd_set_mode(MODE_TRANSFER);

    ASSERT_EQ(read_memory(0x8045), 0xFF);
    ASSERT_EQ(read_memory(0xFF69), 0xFF);
    ASSERT_EQ(read_memory(0xFF68), 0xFF);

    ppu_->tile_data[0x45] = 1;
    write_memory(0x8045, 42);
    ASSERT_EQ(ppu_->tile_data[0x45], 1);
}

TEST_F(OAM, CPU_Mode2)
{
    // During modes 2 and 3, the CPU cannot access OAM ($FE00-FE9F).

    // Set mode 2
    lcd_set_mode(MODE_OAM);

    for (u16 address = 0xFE00; address < 0xFEA0; ++address) {
        ASSERT_EQ(read_memory(address), 0xFF);
        write_oam(address, 1);
        write_memory(address, 42);

        if (address % 4 == 3)
            ASSERT_EQ(ppu_->oam[(address - 0xFE00 - (address % 4)) / 4].raw,
                      0x01010101);
    }
}

TEST_F(OAM, CPU_Mode3)
{
    // During modes 2 and 3, the CPU cannot access OAM ($FE00-FE9F).

    // Set mode 3
    lcd_set_mode(MODE_TRANSFER);

    for (u16 address = 0xFE00; address < 0xFEA0; ++address) {
        ASSERT_EQ(read_memory(address), 0xFF);
        write_oam(address, 1);
        write_memory(address, 42); // Write through CPU -> ignored

        if (address % 4 == 3)
            ASSERT_EQ(ppu_->oam[(address - 0xFE00 - (address % 4)) / 4].raw,
                      0x01010101);
    }
}

} // namespace ppu_test
