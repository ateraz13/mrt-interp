#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

// TODO: Reorder expression tokens into mathematically accurate order.
// NOTE: All intruction are 32-bit but the op code only occupies the first
// 8-bits.

namespace tests {
class InstructionTester;
}

class VirtualMachine {
  friend class tests::InstructionTester;

public:
  using MemPtr = uint32_t;
  using RegID = uint8_t; // Register ID

  void reset() { m_interp.reset(); }

  struct Instruction {
// NOTE: Assign instruction opcodes with specific values.
// TODO: We might need byte, half world operations as well,
// so far we only have word operations
#include "instructions.hxx"
  };

public:
  struct Interpreter {
    void reset() { m_mb.clear(); }

    struct MemoryBank {
    public:
      constexpr static uint64_t MEMORY_SIZE = 64 * 1024; // 64 kilobytes
      // NOTE: Stack grows down
      constexpr static uint64_t STACK_LOWER_LIMIT = 0; // 64 kilobytes
      constexpr static uint64_t STACK_UPPER_LIMIT =
          MEMORY_SIZE / 2; // 64 kilobytes

      // NOTE: It might be a good idea to store memory buffers as uint32_t and
      // values in aligned memory for performance puposes but for now it is
      // unnecessary.
      //
      // NOTE: We can store everything in a sigle memory buffer and grow stack
      // in one direction and heap in the other. just like we do on hardware.
      //
      // NOTE: It might be a good idea to use virtual ROM's to store programs
      // and when neccessary load parts of the program into the working memory.
      //
      // NOTE: Heap allocation will be handled by the kernel.
      //
      // FIXME: Memory Alignment: Most CPU's don't access memory byte at a time
      // but rather in "words". We need a way to keep the memory accessible byte
      // at a time but also be able to store words from and to registers
      // quickly.

      const static uint32_t GP_REGS_32_COUNT = 16;
      const static uint32_t FL_REGS_32_COUNT = 16;

      std::array<uint32_t, GP_REGS_32_COUNT>
          gp_regs_32; /* 15 general purpose 32-bit registers */
      std::array<float, FL_REGS_32_COUNT>
          fl_regs_32; /* 15 floating-point 32-bit registers */
      std::array<uint8_t, MEMORY_SIZE> memory;

      static const RegID GP_A = 0;
      static const RegID GP_B = 1;
      static const RegID GP_C = 2;
      static const RegID GP_D = 3;
      static const RegID GP_E = 4;
      static const RegID GP_F = 5;
      static const RegID STACK_PTR_REG = 12;
      static const RegID PROGRAM_COUNTER_REG = 13;
      static const RegID FLAGS_REG = 14;

      static const uint32_t ZERO_FLAG_BIT = (1 << 0);
      static const uint32_t OVERFLOW_FLAG_BIT = (1 << 1);
      static const uint32_t SIGN_FLAG_BIT = (1 << 2);
      static const uint32_t CARRY_FLAG_BIT = (1 << 3);

      MemoryBank() : gp_regs_32({0}), fl_regs_32({0.0f}), memory({0}) {
        gp_regs_32[PROGRAM_COUNTER_REG] = 0;
        gp_regs_32[STACK_PTR_REG] = STACK_UPPER_LIMIT;
      }

      void clear() {
        std::fill(gp_regs_32.begin(), gp_regs_32.end(), 0);
        std::fill(fl_regs_32.begin(), fl_regs_32.end(), 0);
        fl_regs_32[PROGRAM_COUNTER_REG] = 0;
        gp_regs_32[STACK_PTR_REG] = STACK_UPPER_LIMIT;
        std::fill(memory.begin(), memory.end(), 0);
      }

      void push_register_to_stack(RegID rid) {
        // FIXME: MAybe validate regid here
        if (gp_regs_32[STACK_PTR_REG] <= STACK_LOWER_LIMIT) {
          throw std::runtime_error("Stack underflow!");
        }
        memory[gp_regs_32[STACK_PTR_REG]] = gp_regs_32[rid];
        gp_regs_32[STACK_PTR_REG]--;
      }

      void pop_register_to_stack(RegID rid) {
        // FIXME: MAybe validate regid here
        if (gp_regs_32[STACK_PTR_REG] >= STACK_UPPER_LIMIT) {
          throw std::runtime_error("Stack overflow!");
        }

        gp_regs_32[rid] = memory[gp_regs_32[STACK_PTR_REG]];
        gp_regs_32[STACK_PTR_REG]++;
      }

      bool check_flag(uint32_t flag_bit) const {
        return gp_regs_32[FLAGS_REG] & flag_bit;
      }

      void set_flag(uint32_t flag_bit) { gp_regs_32[FLAGS_REG] |= flag_bit; }

      void unset_flag(uint32_t flag_bit) { gp_regs_32[FLAGS_REG] ^= flag_bit; }

      void print_registers() const {
        for (uint32_t i = 0; i < GP_REGS_32_COUNT; i++) {
          std::cout << "REG" << i << ": " << gp_regs_32[i] << std::endl;
        }

        std::cout << "STACK_PTR: " << gp_regs_32[STACK_PTR_REG] << std::endl;
        std::cout << "PROGRAM_COUNTER: " << gp_regs_32[PROGRAM_COUNTER_REG]
                  << std::endl;
        std::cout << "FLAGS: "
                  << std::bitset<32>(gp_regs_32[PROGRAM_COUNTER_REG])
                  << std::endl;

        std::cout << "\n";

        for (uint32_t i = 0; i < FL_REGS_32_COUNT; i++) {
          std::cout << "FL_REG" << i << ": " << fl_regs_32[i] << std::endl;
        }
      }
    };

    using BytecodeBuffer = std::vector<uint8_t>;
    // NOTE: The program can be excecuted in terms of reading byte at
    // a time and keeping a state, interpretting subsequent bytes based
    // on opcodes and instruction specs, this way we can keep immediate values
    // in the bytecode.
    // NOTE: We need to figure out how we're going to store immediate values
    void load_program(BytecodeBuffer &buffer);
    void run();

  private:
    using MemoryBuffer = decltype(MemoryBank::memory);

    RegID read_valid_regid(MemoryBuffer &buffer, uint32_t &pc) const;
    RegID read_valid_float_regid(MemoryBuffer &buffer, uint32_t &pc) const;
    MemPtr read_valid_mem_address(MemoryBuffer &buffer, uint32_t &pc) const;
    int read_valid_int_immediate_val(MemoryBuffer &buffer, uint32_t &pc) const;
    float read_valid_float_immediate_val(MemoryBuffer &buffer,
                                         uint32_t &pc) const;
    MemPtr check_mem_address_with_throw(MemPtr ptr) const;
    void check_bytes_ahead(MemoryBuffer &buffer, size_t index,
                           size_t count) const;
    void check_jump_address(RegID regid) const;

    void push_stack(RegID reg);
    void pop_stack(RegID reg);
    void load(RegID reg, MemPtr ptr);
    void store(RegID reg, MemPtr ptr);
    void load_immediate(RegID reg, uint32_t val);
    void store_float(RegID reg, MemPtr ptr);
    void load_float(RegID reg, MemPtr ptr);
    void load_immediate_float(RegID reg, float val);

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

    // FIXME: In the future we want to make this private
  public:
    MemoryBank m_mb;
  };

  // FIXME: In the future we want to make this private
public:
  Interpreter m_interp;
};

#endif // INTERPRETER_H
