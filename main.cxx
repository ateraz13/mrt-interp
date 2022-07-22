#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include "parse.hxx"
#include "syntax.hxx"
#include <list>

// TODO: Reorder expression tokens into mathematically accurate order.

struct RegisterBank {

};


struct MemoryBank {

  using MemPtr = uint64_t;
  using RegID = uint8_t; // Register ID
  constexpr static uint64_t MEMORY_SIZE = 64*1024; // 64 kilobytes

  // NOTE: It might be a good idea to store memory buffers as uint32_t and values in aligned
  // memory for performance puposes but for now it is unnecessary.
  //
  // NOTE: We can store everything in a sigle memory buffer and grow stack in one direction and heap in the other.
  // just like we do it on the hardware.

  std::array<uint32_t, 15> regs_32; /* 15 general purpose 32-bit registers */
  std::array<float, 15> fl_reg_32; /* 15 floating-point 32-bit registers */
  std::array<uint32_t, MEMORY_SIZE> memory;

  MemPtr stack_ptr;
  MemPtr program_counter;

  void push_stack(RegID reg, int val);
  void pop_stack(RegID reg);

  void store_to_mem(RegID reg, MemPtr ptr);
  void load_from_mem(RegID reg, MemPtr ptr);

  void store_float_to_mem(RegID reg, MemPtr ptr);
  void load_float_from_mem(RegID reg, MemPtr ptr);

  void literal_to_reg(int32_t val, MemPtr ptr);
  void literal_to_float_reg(int32_t val, MemPtr ptr);

  void add_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);
  void sub_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);
  void mult_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);
  void div_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);
  void shift_left_(RegID reg1, RegID shit_by_reg2, RegID dest_reg);
  void shift_right(RegID reg1, RegID shit_by_reg2, RegID dest_reg);
  void shift_left_literal(RegID reg1, uint32_t shift_by, RegID dest_reg);
  void shift_right_literal(RegID reg1, uint32_t shift_by, RegID dest_reg);

  void add_float_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);
  void sub_float_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);
  void mult_float_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);
  void div_float_regs_to_reg(RegID reg1, RegID reg2, RegID dest_reg);

  void jump(RegID reg);
  void jump_zero(RegID reg);
  void jump_eq(RegID reg);
  void jump_lt(RegID reg);
  void jump_gt(RegID reg);

};

// NOTE: Instruction can be stored as a function pointers in an associative
// array and excecuted there after by indexing into the array
// and calling the function the pointer is pointing to.

struct Instruction
{
  // NOTE: Assign instruction opcodes with specific values.
  enum struct OpCode {
    PLUS_INT, MINUS_INT, DIV_INT, MULT_INT,
    PLUS_LITERAL_INT, MINUS_LITERAL_INT, DIV_LITERAL_INT, MULT_LITERAL_INT,
    SHIFT_LEFT, SHIFT_RIGHT, SHIFT_LITERAL_LEFT, SHIFT_LITERAL_RIGHT,
    PLUS_FLOAT, MINUS_FLOAT, DIV_FLOAT, MULT_FLOAT,
    PLUS_LITERAL_FLOAT, MINUS_LITERAL_FLOAT, DIV_LITERAL_FLOAT, MULT_LITERAL_FLOAT,
    PUSH_STACK, POP_STACK, LOAD_REGISTER, STORE_REGISTER,
    PUSH_FLOAT_STACK, POP_FLOAT_STACK, LOAD_FLOAT_REGISTER, STORE_FLOAT_REGISTER,
    JUMP, JUMP_ZERO, JUMP_EQ, JUMP_LT, JUMP_GT,
    COMPARE, COMPARE_FLOAT,
  } op_code;

};

class Interpreter {

};

int main(int argc, char** argv)
{
  std::cout << "Hello there!" << std::endl;



  std::cout << "Press any key to exit..." << std::endl;
  getchar();

  return 0;
}
