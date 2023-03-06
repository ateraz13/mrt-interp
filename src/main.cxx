#include "instructions.hxx"
#include "interpreter.hxx"
#include "parse.hxx"
#include "syntax.hxx"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <list>
#include <sstream>
#include <string>

/*/ NOTE: Instruction can be stored as a function pointers in an associative
*** array and excecuted there after by indexing into the array
*** and calling the function the pointer is pointing to.
***
*** TODO: Write dissassebler to analyze memory contents better.
***  When we analyze instructions in memory they are printed out as raw bytes,
***  we need an dissasable to make it easier for us to analyze the code.
***
*** TODO: Implement BytecodeBuffer builder class.
*** + We want code generator functions
*** + It doen't neccerraly have to be a seperate class from BytecodeBuffer,
***   because we may want to refill the buffer with different intructions
***   several times.
*** + Example:
***      bbb.begin()
***         .instructions(
***             {
***                Some bytecode ...
***             }
***         )
***         .gen_function(
***            []() {
***               Generate some bytecode ...
***            }
***          )
***         .instructions(
***             []() {
***                Some more instructions ...
***             }
***         )
***         .end();
***
*** TODO: Use a variant to store instruction paraments
*** and each callback to a instruction implementation will take a
*** it as a parameter. This means we can store all the callbacks in
*** a vector and than call them from the interpreter runtime.
/*/

#define ADDR(addr)                                                             \
  (addr & 0xff000000) << 0, (addr & 0x00ff0000) >> 8,                          \
      (addr & 0x0000ff00) >> 16, (addr & 0x000000ff) >> 24

#define REG(r) r

#define IMM(imm)                                                               \
  (imm & 0xff000000) << 0, (imm & 0x00ff0000) >> 8, (imm & 0x0000ff00) >> 16,  \
      (imm & 0x000000ff) >> 24

int main(int argc, char **argv) {
  using OPC = VM::OpCodes;

  try {

    const uint8_t addr[4]{ADDR(0xfaff)};
    std::cout << "ADDR: " << std::hex << (int)addr[0] << std::hex
              << (int)addr[1] << std::hex << (int)addr[2] << std::hex
              << (int)addr[3] << std::endl;

    std::cout << *(int *)addr << std::endl;

    // clang-format off
    VirtualMachine vm;
    Interpreter::BytecodeBuffer bb{
            OPC::LOAD_IMMEDIATE, IMM(5), REG(1),
            OPC::LOAD_IMMEDIATE,  IMM(7), REG(2),
            OPC::ADD_INT, REG(1), REG(2), REG(1),
            OPC::STORE, REG(1), ADDR(0xff),
            OPC::LOAD_FLOAT_IMMEDIATE, REG(1), IMM(0),
            OPC::HALT
    };
    // clang-format on

    vm.m_interp.start();
    vm.m_interp.load_program(bb);
    vm.m_interp.run();

    std::cout << "Hello there!" << std::endl;

    vm.m_interp.m_mb.print_registers();

    std::cout << "Press any key to exit..." << std::endl;
    getchar();
  } catch (std::runtime_error re) {
    std::cout << "RUNTIME_ERROR: " << re.what() << std::endl;
  }

  return 0;
}
