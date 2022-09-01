// REG_ERR is also defined inside gtest ...
#include "CPU/instruction.h"
#define REG_ERR REG_ERR_
#include <gtest/gtest.h>
#undef REG_ERR

extern "C" {
#include <CPU/flag.h>
#include <CPU/timer.h>
#include <options.h>
}

#include "../../cartridges/cartridge.hxx"

#define CASES(prefix_, struct_) const std::vector<struct_> prefix_##_##struct_

struct timer {
    u16 div;
    u8 tima;
    u8 tma;
    u8 tac;
};

extern struct timer timer;

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

        reset_timer();
    }

    virtual void TearDown() override
    {
        if (!loaded_)
            return;

        ASSERT_EQ(cpu.registers.pc, start_pc_ + size_);
        ASSERT_EQ(timer.div, cycles_);
    }

  private:
    const u8 size_;
    const u16 start_pc_ = 0x1000;

  protected:
    u8 cycles_ = 0;
    bool loaded_ = false;

    void Load(void *instruction)
    {
        memcpy(&cartridge.rom[cpu.registers.pc], instruction, size_);

        const auto pc = cpu.registers.pc;
        cycles_ = fetch_instruction(fetch_opcode()).cycle_count;
        loaded_ = true;
        timer.div = 0;
    }
};
