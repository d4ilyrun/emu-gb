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

struct in_type {
    in_name name;
    operand_type type;
    u8 cycle_count;
    u8 cycle_count_false; // For conditional jumps
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
        static u8 codes[256] = {
            1, 3, 2, 2, 2, 2, 2, 1, 5, 2, 2, 2, 2, 2, 2, 1, 1, 3, 2, 2, 2, 2,
            2, 1, 3, 2, 2, 2, 2, 2, 2, 1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2,
            2, 2, 2, 1, 2, 3, 2, 2, 2, 2, 3, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1,
            1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
            1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1,
            2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1,
            1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
            1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
            1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 3, 3, 4, 3, 4,
            2, 0, 2, 4, 3, 1, 3, 6, 2, 4, 2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0,
            3, 0, 2, 4, 3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4, 3, 3,
            2, 1, 0, 4, 2, 4, 4, 2, 4, 1, 0, 0, 2, 4,
        };

        memcpy(&cartridge.rom[cpu.registers.pc], instruction, size_);
        cycles_ = codes[*((u8 *)instruction)];
        loaded_ = true;
        timer.div = 0;
    }
};
