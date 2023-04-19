#include <interp/test_instructions.hxx>

int main(int argc, char** argv)
{
  tests::InstructionTester instruction_tester;
  instruction_tester.run_tests();

  return 0;
}
