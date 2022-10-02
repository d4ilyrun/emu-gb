// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
#include "ppu/lcd.h"
}

namespace ppu_test
{

typedef shade palette[4];

static palette default_palette = {
    0xFFFFFFFF, // White
    0xFFAAAAAA, // Light grey
    0xFF555555, // Dark grey
    0xFF000000, // Black
};

class LCDTest : public ::testing::Test
{
  public:
    LCDTest() {}

    void SetUp() override
    {
        init_lcd();
    }
};

using Registers = LCDTest;
using Palette = LCDTest;
using DMA = LCDTest;

TEST_F(Registers, Default)
{
    const auto lcd = get_lcd();

    ASSERT_EQ(lcd->lcdc, 0x91);

    ASSERT_EQ(lcd->scx, 0);
    ASSERT_EQ(lcd->scy, 0);
    ASSERT_EQ(lcd->wx, 0);
    ASSERT_EQ(lcd->wy, 0);

    ASSERT_EQ(lcd->ly, 0);
    ASSERT_EQ(lcd->lyc, 0);
}

TEST_F(Registers, StatUpdateLY)
{
    const auto lcd = get_lcd();

    ASSERT_TRUE(LCD_STAT_LY_FLAG(*lcd));

    write_lcd(0xFF45, 1);
    ASSERT_FALSE(LCD_STAT_LY_FLAG(*lcd));

    write_lcd(0xFF44, 1);
    ASSERT_TRUE(LCD_STAT_LY_FLAG(*lcd));
}

TEST_F(Palette, Default)
{
    const auto reg = get_lcd()->dmg;

    ASSERT_EQ(reg.bgp, 0xFC);
    ASSERT_EQ(reg.obp[0], 0xFF);
    ASSERT_EQ(reg.obp[1], 0xFF);

    for (int i = BG_PALETTE; i < INVALID; ++i) {
        const auto palette = lcd_get_palette((palette_name)i);

        ASSERT_EQ(palette[0], default_palette[0]);
        ASSERT_EQ(palette[1], default_palette[1]);
        ASSERT_EQ(palette[2], default_palette[2]);
        ASSERT_EQ(palette[3], default_palette[3]);
    }
}

TEST_F(Palette, Background)
{
    const auto reg = get_lcd()->dmg;
    const auto palette = lcd_get_palette(BG_PALETTE);

    write_lcd(0xFF47, 0b11001001);

    ASSERT_EQ(palette[0], default_palette[0b01]);
    ASSERT_EQ(palette[1], default_palette[0b10]);
    ASSERT_EQ(palette[2], default_palette[0b00]);
    ASSERT_EQ(palette[3], default_palette[0b11]);

    ASSERT_EQ(reg.bgp, 0b11001001);
}

TEST_F(Palette, Sprite)
{
    const auto reg = get_lcd()->dmg;
    const auto palette = lcd_get_palette(SPRITE_PALETTE_0);

    write_lcd(0xFF48, 0b10110101);

    ASSERT_EQ(palette[0], default_palette[0b01]);
    ASSERT_EQ(palette[1], default_palette[0b01]);
    ASSERT_EQ(palette[2], default_palette[0b11]);

    // 2 lower bits are ignored
    ASSERT_EQ(palette[3], default_palette[3]);

    ASSERT_EQ(reg.obp[0], 0b11001001);
}

} // namespace ppu_test
