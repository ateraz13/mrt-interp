#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <array>
#include <vector>
#include <cstdint>
#include <stdexcept>

// TODO: Reorder expression tokens into mathematically accurate order.
// NOTE: All intruction are 32-bit but the op code only occupies the first 8-bits.

class VirtualMachine
{
public:
  using MemPtr = uint64_t;
  using RegID = uint8_t; // Register ID

  struct Instruction
  {
    // NOTE: Assign instruction opcodes with specific values.
    struct OpCodes {

      static const uint8_t PUSH_STACK = 0;
      static const uint8_t POP_STACK = 1;

      static const uint8_t PUSH_FLOAT_STACK = 2;
      static const uint8_t POP_FLOAT_STACK = 3;

      static const uint8_t LOAD = 4;
      static const uint8_t LOAD_IMMEDIATE = 5;

      static const uint8_t LOAD_FLOAT = 6;
      static const uint8_t LOAD_IMMEDIATE_FLOAT = 7;

      static const uint8_t STORE_MEMORY = 8;
      static const uint8_t STORE_FLOAT = 9;

      static const uint8_t SHIFT_LEFT = 10;
      static const uint8_t SHIFT_RIGHT = 11;
      static const uint8_t SHIFT_IMMEDIATE_LEFT = 12;
      static const uint8_t SHIFT_IMMEDIATE_RIGHT = 13;

      static const uint8_t ADD_INT = 14;
      static const uint8_t SUB_INT = 15;
      static const uint8_t DIV_INT = 16;
      static const uint8_t MULT_INT = 17;

      static const uint8_t PLUS_IMMEDIATE_INT = 18;
      static const uint8_t MINUS_IMMEDIATE_INT = 19;
      static const uint8_t DIV_IMMEDIATE_INT = 20;
      static const uint8_t MULT_IMMEDIATE_INT = 21;

      static const uint8_t ADD_FLOAT = 22;
      static const uint8_t SUB_FLOAT = 23;
      static const uint8_t DIV_FLOAT = 24;
      static const uint8_t MULT_FLOAT = 25;

      static const uint8_t PLUS_IMMEDIATE_FLOAT = 26;
      static const uint8_t MINUS_IMMEDIATE_FLOAT = 27;
      static const uint8_t DIV_IMMEDIATE_FLOAT = 28;
      static const uint8_t MULT_IMMEDIATE_FLOAT = 29;

      static const uint8_t JUMP = 30;
      static const uint8_t JUMP_ZERO = 31;
      static const uint8_t JUMP_EQ = 32;
      static const uint8_t JUMP_LT = 33;
      static const uint8_t JUMP_GT = 34;

      static const uint8_t COMPARE = 35;
      static const uint8_t COMPARE_FLOAT = 36;

    } op_code;

  };

public:
  struct Interpreter
  {
    struct MemoryBank
    {
    public:
      constexpr static uint64_t MEMORY_SIZE = 64*1024; // 64 kilobytes
      // NOTE: Stack grows down
      constexpr static uint64_t STACK_LOWER_LIMIT = 0; // 64 kilobytes
      constexpr static uint64_t STACK_UPPER_LIMIT = MEMORY_SIZE/2; // 64 kilobytes

      // NOTE: It might be a good idea to store memory buffers as uint32_t and values in aligned
      // memory for performance puposes but for now it is unnecessary.
      //
      // NOTE: We can store everything in a sigle memory buffer and grow stack in one direction and heap in the other.
      // just like we do on hardware.

      std::array<uint32_t, 15> gp_regs_32; /* 15 general purpose 32-bit registers */
      std::array<float, 15> fl_reg_32; /* 15 floating-point 32-bit registers */
      std::array<uint32_t, MEMORY_SIZE> memory;

      static const RegID GP_A = 0;
      static const RegID GP_B = 1;
      static const RegID GP_C = 2;
      static const RegID GP_D = 3;
      static const RegID GP_E = 4;
      static const RegID GP_F = 5;
      static const RegID FLAGS_REG = 14;

      MemPtr stack_ptr;
      MemPtr program_counter;

      void push_register_to_stack(RegID rid) {
        //FIXME: MAybe validate regid here
        if(stack_ptr <= STACK_LOWER_LIMIT) {
          throw std::runtime_error("Stack underflow!");
        }
        memory[stack_ptr] = gp_regs_32[rid];
        stack_ptr--;
      }

      void pop_register_to_stack(RegID rid) {
        //FIXME: MAybe validate regid here
        if(stack_ptr >= STACK_UPPER_LIMIT) {
          throw std::runtime_error("Stack overflow!");
        }

        gp_regs_32[rid] = memory[stack_ptr];
        stack_ptr++;
      }

    };

    using BytecodeBuffer = std::vector<uint8_t>;

    // NOTE: The program can be excecuted in terms of reading byte at
    // a time and keeping a state, interpretting subsequent bytes based
    // on opcodes and instruction specs, this way we can keep immediate values
    // in the bytecode.
    // NOTE: We need to figure out how we're going to store immediate values
    void execute(BytecodeBuffer &buffer);

  private:

    RegID read_valid_regid(RegID rid) const;
    RegID read_valid_float_regid(RegID rid) const;
    void check_bytes_ahead(BytecodeBuffer &buffer, size_t index, size_t count) const;
    MemPtr read_valid_mem_address(BytecodeBuffer &buffer, size_t index) const;
    int read_valid_int_immediate_val(BytecodeBuffer &buffer, size_t index) const;
    float read_valid_float_immediate_val(BytecodeBuffer &buffer, size_t index) const;

    void push_stack(RegID reg);
    void pop_stack(RegID reg);

    void load(RegID reg, MemPtr ptr);
    void store(RegID reg, MemPtr ptr);
    void load_immediate(uint32_t val, RegID reg);
    void store_float(RegID reg, MemPtr ptr);
    void load_float(RegID reg, MemPtr ptr);
    void load_immediate_float(float val, RegID reg);

    void shift_left(RegID reg1, RegID shit_by_reg2, RegID dest_reg);
    void shift_right(RegID reg1, RegID shit_by_reg2, RegID dest_reg);
    void shift_left_immediate(RegID reg1, uint32_t shift_by, RegID dest_reg);
    void shift_right_immediate(RegID reg1, uint32_t shift_by, RegID dest_reg);

    void add(RegID reg1, RegID reg2, RegID dest_reg);
    void sub(RegID reg1, RegID reg2, RegID dest_reg);
    void mult(RegID reg1, RegID reg2, RegID dest_reg);
    void div(RegID reg1, RegID reg2, RegID dest_reg);

    void add_float(RegID reg1, RegID reg2, RegID dest_reg);
    void sub_float(RegID reg1, RegID reg2, RegID dest_reg);
    void mult_float(RegID reg1, RegID reg2, RegID dest_reg);
    void div_float(RegID reg1, RegID reg2, RegID dest_reg);

    void jump(RegID reg);
    void jump_zero(RegID reg);
    void jump_eq(RegID reg);
    void jump_lt(RegID reg);
    void jump_gt(RegID reg);

    void cmp(RegID reg1, RegID reg2);
    void cmp_float(RegID reg1, RegID reg2);

    MemoryBank m_mb;
  };

private:
  Interpreter m_exec_ng;
};

#endif // INTERPRETER_H