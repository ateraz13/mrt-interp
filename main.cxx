#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <list>
#include "parse.hxx"
#include "syntax.hxx"
#include "interpreter.hxx"

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
/*/

int main(int argc, char** argv)
{
  using OPC = VirtualMachine::Instruction::OpCodes;

  try {

    VirtualMachine vm;
    VirtualMachine::Interpreter::BytecodeBuffer bb{
        OPC::LOAD_IMMEDIATE, 0x01 /* REG1 */, 0x05, 0x00, 0x00, 0x00,
        OPC::LOAD_IMMEDIATE, 0x02 /* REG1 */, 0x07, 0x00, 0x00, 0x00,
        OPC::ADD_INT, 0x1, 0x2, 0x1,                         /* REG1 += REG2 */
        OPC::STORE, 0x01, /* REG1 */ 0xff, 0x00, 0x00, 0x00, /* ADDR: 255 = 0xff */

        OPC::LOAD_IMMEDIATE_FLOAT, 0x00 /* FL_REG1 */, 0xff, 0xff, 0xff, 0xff,
        OPC::HALT};

    vm.m_interp.load_program(bb);
    vm.m_interp.run();

    std::cout << "Hello there!" << std::endl;

    vm.m_interp.m_mb.print_registers();

    std::cout << "Press any key to exit..." << std::endl;
    getchar();
  }
  catch (std::runtime_error re)
  {
    std::cout << "RUNTIME_ERROR: " << re.what()<< std::endl;
  }

  return 0;
}
