#include "../instruction.hxx"

namespace cpu_tests
{

template <u8 size, typename param_type>
class LoadInstructionTest : public InstructionTest,
                            public ::testing::WithParamInterface<param_type>
{
  public:
    LoadInstructionTest() : InstructionTest(size) {}
    virtual void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
    }

    virtual void TearDown() override
    {
        InstructionTest::TearDown();
        const auto &param = this->GetParam();
        ASSERT_EQ(param.expected, *param.out);
    }
};

template <u8 size, typename param_type>
class LoadInstruction16BitTest : public LoadInstructionTest<size, param_type>
{
  public:
    void TearDown() override
    {
        InstructionTest::TearDown();
        const auto &param = this->GetParam();
        ASSERT_EQ(Get16Bit(param.expected), *param.out + *(param.out + 1));
    }

    virtual void SetUp() override
    {
        InstructionTest::SetUp();
        const auto &param = this->GetParam();
        if constexpr (size == 3) { // Load immediate 16bit value
            const auto &instruction = param.instruction;
            const u16 word = Get16Bit(instruction[1]);
            memcpy(&cartridge.rom[cpu.registers.pc], instruction, 1);
            memcpy(&cartridge.rom[cpu.registers.pc + 1], &word, 2);
        } else {
            ASSERT_EQ(Get16Bit(param.expected), *param.out + *(param.out + 1));
        }
    };

  protected:
    constexpr u16 Get16Bit(u8 value)
    {
        // Reverse one nybble and combine
        return (value << 8) | (value & 0xF) << 4 | (value & 0xF0) >> 4;
    }
};

} // namespace cpu_tests
