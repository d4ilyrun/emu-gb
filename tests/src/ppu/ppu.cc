// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <gtest/gtest_pred_impl.h>
#undef REG_ERR

extern "C" {
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

using VRAM = PPUTest;
using OAM = PPUTest;

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

} // namespace ppu_test
