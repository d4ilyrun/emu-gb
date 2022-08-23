// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <CPU/flag.h>
}

#include "../../cartridges/cartridge.hxx"

class InstructionTest : public ::testing::Test
{
  public:
    InstructionTest(u8 size) : size_(size)
    {
        cartridge = CartridgeGenerator<1 << 18>(ROM_ONLY).GetCart();
    }

    virtual void SetUp() override
    {
        cpu.registers.pc = start_pc_;
        set_all_flags(false, false, false, false);
    }

    virtual void TearDown() override
    {
        ASSERT_EQ(cpu.registers.pc, start_pc_ + size_);
    }

  private:
    const u8 size_;
    const u16 start_pc_ = 0x1000;

  protected:
    void Load(void *instruction)
    {
        memcpy(&cartridge.rom[start_pc_], instruction, size_);
    }
};
