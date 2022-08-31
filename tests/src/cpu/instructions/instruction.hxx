// REG_ERR is also defined inside gtest ...
#define REG_ERR REG_ERR_
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <CPU/flag.h>
#include <options.h>
}

#include "../../cartridges/cartridge.hxx"

#define CASES(prefix_, struct_) const std::vector<struct_> prefix_##_##struct_

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
        get_options()->trace = false; // Manually set the value when needed
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
        memcpy(&cartridge.rom[cpu.registers.pc], instruction, size_);
    }
};
