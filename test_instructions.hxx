#ifndef TEST_INSTRUCTIONS
#define TEST_INSTRUCTIONS
#define FAIL_TEST(msg) return {msg};
#define NOT_IMPLEMENTED FAIL_TEST("Test not implemented");
#include "interpreter.hxx"
#include <boost/format.hpp>
#include <functional>
#include <optional>
#include <string>

// NOTE: Implement interface for Testers so that we can use the
// same technique for other part of the program. This way testing the program
// could be automated with a common interface.
// FIXME: Handle Endianness

namespace tests {
class InstructionTester {
public:
  using TestError = std::optional<std::string>;

  struct Test {
    const char *name;
    std::function<std::vector<TestError>(VirtualMachine &)> test_func;
  };

#define LITTLE_U32(a, b, c, d) d, c, b, a
  std::vector<Test> tests{
      {"test_mem_access_instructions",
       [](VirtualMachine &vm) -> std::vector<TestError> {
         using OPS = VM::OpCodes;
         vm.reset();

         const uint32_t base_addr = 0x0100;
         std::vector<TestError> test_errors;

         std::cout << "Testing Loading Immediate instructions... \n";

         Interpreter::BytecodeBuffer bb{OPS::LOAD_IMMEDIATE,
                                        0x00,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x00),
                                        OPS::LOAD_IMMEDIATE,
                                        0x01,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x01),
                                        OPS::LOAD_IMMEDIATE,
                                        0x02,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x02),
                                        OPS::LOAD_IMMEDIATE,
                                        0x03,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x03),
                                        OPS::LOAD_IMMEDIATE,
                                        0x04,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x04),
                                        OPS::LOAD_IMMEDIATE,
                                        0x05,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x05),
                                        OPS::LOAD_IMMEDIATE,
                                        0x06,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x06),
                                        OPS::LOAD_IMMEDIATE,
                                        0x07,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x07),
                                        OPS::LOAD_IMMEDIATE,
                                        0x08,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x08),
                                        OPS::LOAD_IMMEDIATE,
                                        0x09,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x09),
                                        OPS::LOAD_IMMEDIATE,
                                        0x0a,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x0a),
                                        OPS::LOAD_IMMEDIATE,
                                        0x0b,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x0b),
                                        OPS::LOAD_IMMEDIATE,
                                        0x0c,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x0c),
                                        OPS::LOAD_IMMEDIATE,
                                        0x0d,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x0d),
                                        OPS::LOAD_IMMEDIATE,
                                        0x0e,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x0e),
                                        OPS::LOAD_IMMEDIATE,
                                        0x0f,
                                        LITTLE_U32(0xff, 0x00, 0x00, 0x0f),
                                        OPS::HALT};

         vm.m_interp.load_program(bb);
         vm.m_interp.run();

         for (uint32_t ri = 0; ri < MemoryBank::GP_REGS_32_COUNT; ri++) {
           if (vm.m_interp.m_mb.gp_regs_32[ri] != (0xff000000 + ri)) {
             auto err_msg =
                 (boost::format("Invalid value found in register:\n\t"
                                "Expected: %1%\n\t"
                                "Found: %2%\n") %
                  (0xff000000 + ri) % vm.m_interp.m_mb.gp_regs_32[ri])
                     .str();
             test_errors.push_back(err_msg);
           }
         }

         std::cout << "Finished testing load immediate instruction.\n";
         std::cout << "Testing load instructions...\n";

         bb = {OPS::LOAD, 0x00, LITTLE_U32(0x00, 0x00, 0x01, 0x00),
               OPS::LOAD, 0x01, LITTLE_U32(0x00, 0x00, 0x01, 0x01),
               OPS::LOAD, 0x02, LITTLE_U32(0x00, 0x00, 0x01, 0x02),
               OPS::LOAD, 0x03, LITTLE_U32(0x00, 0x00, 0x01, 0x03),
               OPS::LOAD, 0x04, LITTLE_U32(0x00, 0x00, 0x01, 0x04),
               OPS::LOAD, 0x05, LITTLE_U32(0x00, 0x00, 0x01, 0x05),
               OPS::LOAD, 0x06, LITTLE_U32(0x00, 0x00, 0x01, 0x06),
               OPS::LOAD, 0x07, LITTLE_U32(0x00, 0x00, 0x01, 0x07),
               OPS::LOAD, 0x08, LITTLE_U32(0x00, 0x00, 0x01, 0x08),
               OPS::LOAD, 0x09, LITTLE_U32(0x00, 0x00, 0x01, 0x09),
               OPS::LOAD, 0x0a, LITTLE_U32(0x00, 0x00, 0x01, 0x0a),
               OPS::LOAD, 0x0b, LITTLE_U32(0x00, 0x00, 0x01, 0x0b),
               OPS::LOAD, 0x0c, LITTLE_U32(0x00, 0x00, 0x01, 0x0c),
               OPS::LOAD, 0x0d, LITTLE_U32(0x00, 0x00, 0x01, 0x0d),
               OPS::LOAD, 0x0e, LITTLE_U32(0x00, 0x00, 0x01, 0x0e),
               OPS::LOAD, 0x0f, LITTLE_U32(0x00, 0x00, 0x01, 0x0f),
               OPS::HALT};

         vm.reset();

         // NOTE: We need to initilize the memory to contain the test values
         for (uint32_t i = 0; i < 16; i++) {
           uint32_t addr = base_addr + i;
           vm.m_interp.m_mb.memory[addr] = i;
         }

         vm.m_interp.load_program(bb);
         vm.m_interp.run();

         for (uint32_t addr_offset = 0; addr_offset < 16; addr_offset++) {
           uint32_t addr = base_addr + addr_offset;
           if (vm.m_interp.m_mb.memory[addr] != addr_offset) {
             auto err_msg = (boost::format("Invalid value at address: %1%\n"
                                           "Expecting: %2%\n"
                                           "Found: %3%\n") %
                             addr % addr_offset % vm.m_interp.m_mb.memory[addr])
                                .str();
             test_errors.push_back(err_msg);
           }
         }

         std::cout << "Finished testing load instructions...";
         vm.reset();
         return {};
       }},
      {"test_arithmetic_instructions",
       [](VirtualMachine &vm) -> std::vector<TestError> { NOT_IMPLEMENTED; }},
      {"test_jump_instructions",
       [](VirtualMachine &vm) -> std::vector<TestError> { NOT_IMPLEMENTED; }},
      {"test_compare_instructions",
       [](VirtualMachine &vm) -> std::vector<TestError> { NOT_IMPLEMENTED; }},
      {"test_auxilary_instructions",
       [](VirtualMachine &vm) -> std::vector<TestError> { NOT_IMPLEMENTED; }}};
#undef LITTLE_U32

  std::vector<std::pair<const char *, std::vector<TestError>>> run_tests() {
    std::vector<std::pair<const char *, std::vector<TestError>>> results;
    for (auto test : tests) {
      auto res = test.test_func(m_vm);
      std::cout << "Running test: " << test.name;
      if (!res.size()) {
        std::cout << " ---> OK\n";
      } else {
        std::cout << " ---> FAILED\n";
        for (auto err : res) {
          std::cerr << *err << std::endl;
        }
      }
      results.push_back({test.name, res});
    }

    return results;
  }

private:
  VirtualMachine m_vm;
};
} // namespace tests
#undef NOT_IMPLEMENTED
#undef FAIL_TEST
#endif // TEST_INSTRUCTIONS
