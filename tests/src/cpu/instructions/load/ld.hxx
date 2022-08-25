#include "../instruction.hxx"

namespace cpu_tests
{

template <u8 size, typename param_type>
class LoadInstructionTest : public InstructionTest,
                            public ::testing::WithParamInterface<param_type>
{
  public:
    LoadInstructionTest() : InstructionTest(size) {}
    void SetUp() override
    {
        InstructionTest::SetUp();
        Load((void *)this->GetParam().instruction);
    }

    void TearDown() override
    {
        InstructionTest::TearDown();
        const auto &param = this->GetParam();
        ASSERT_EQ(param.expected, *param.out);
    }
};

} // namespace cpu_tests
