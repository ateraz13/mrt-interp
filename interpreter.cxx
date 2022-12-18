#include "interpreter.hxx"
#include "instructions.hxx"
#include <boost/format.hpp>
#include <boost/limits.hpp>
#include <boost/numeric/conversion/converter.hpp>
#include <climits>

using OpCodes = VM::OpCodes;

// NOTE: We need to deal with endienness in immediate values, memory addresses
// etc.

// FIXME: This function does nothing since it was writen on x86 system.
// On big-endian systems it needs to switch the bytes around to match the
// systems endianness since the vm stores numbers in little-endian format.

void Interpreter::load_program(BytecodeBuffer &buffer) {
  if (buffer.size() > m_mb.memory.size()) {
    throw std::runtime_error(
        (boost::format("Program too large for VM memory!(size: %1%)") %
         buffer.size())
            .str());
  }

  // Load the program to address 0
  std::copy(begin(buffer), end(buffer), begin(m_mb.memory));
}

void Interpreter::run() {
  // NOTE: We can copy all the instruction into a local buffer and exit, as the
  // interpreter can run on a seperate thread.
  //
  // NOTE: Program can be loaded to the address 0 in VM memory.
  // NOTE: When we stamble upon a invalid instruction we can raise an interrupt
  // to handle the error. NOTE: Maybe we want a bootloader in order to load a
  // kernel as well as some standard boot process.
  // FIXME: The program is loaded into memory, we're not using bytecode buffers.
  // They are only temporary storage used to store instructions before they are
  // passed to the interpreter.
  auto &pc = m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG];
  auto &mem = m_mb.memory;

  while (pc < mem.size()) {
    VM::run_next_instruction(*this);
  }
}
using Interpreter = Interpreter;

// NOTE: Don't implement everything in the MemoryBank because than we
// unnecessarily have bigger call stack. Just access private members variables
// of the MemoryBank from here. NOTE: It might be more efficient to verify
// memory pointers during decoding in the excecute method. NOTE: Instead of
// interpreting the bytecode directly we can use JIT (Just in Time) compilation
// to native machine code.
// NOTE: Mathematical operations are done in order the values are supplied in.
// TODO: The arithmetic instruction need to raise flags!
// TODO: Go through all the instructions and verify they are implemented
// correctly.

void VM::callbacks::nop_cb(Interpreter &interp) {}

void VM::callbacks::push_cb(Interpreter &interp, RegID regid) {
  interp.m_mb.push_register_to_stack(regid);
}

void VM::callbacks::pushf_cb(Interpreter &interp, FL_RegID regid) {
  interp.m_mb.push_float_register_to_stack(regid);
}

void VM::callbacks::pop_cb(Interpreter &interp, RegID regid) {
  interp.m_mb.pop_register_from_stack(regid);
}

void VM::callbacks::popf_cb(Interpreter &interp, FL_RegID regid) {
  interp.m_mb.pop_float_register_from_stack(regid);
}

void VM::callbacks::ld_cb(Interpreter &interp, MemPtr source,
                          RegID destination) {
  interp.m_mb.gp_regs_32[destination] =
      *reinterpret_cast<uint32_t *>(&interp.m_mb.memory[source]);
}

void VM::callbacks::lb_cb(Interpreter &interp, MemPtr source,
                          RegID destination) {
  interp.m_mb.gp_regs_32[destination] =
      *reinterpret_cast<uint8_t *>(&interp.m_mb.memory[source]);
}

void VM::callbacks::lhw_cb(Interpreter &interp, MemPtr source,
                           RegID destination) {
  interp.m_mb.gp_regs_32[destination] =
      *reinterpret_cast<uint16_t *>(&interp.m_mb.memory[source]);
}

void VM::callbacks::ldi_cb(Interpreter &interp, uint32_t val, RegID regid) {
  interp.m_mb.gp_regs_32[regid] = val;
}

void VM::callbacks::lbi_cb(Interpreter &interp, uint8_t val, RegID regid) {
  interp.m_mb.gp_regs_32[regid] = val;
}

void VM::callbacks::lhwi_cb(Interpreter &interp, uint16_t val, RegID regid) {
  interp.m_mb.gp_regs_32[regid] = val;
}

void VM::callbacks::lf_cb(Interpreter &interp, MemPtr ptr, RegID regid) {
  interp.m_mb.fl_regs_32[regid] = interp.m_mb.memory[ptr];
}

void VM::callbacks::lfi_cb(Interpreter &interp, float val, RegID regid) {
  interp.m_mb.fl_regs_32[regid] = val;
}

void VM::callbacks::st_cb(Interpreter &interp, RegID regid, MemPtr ptr) {
  interp.m_mb.memory[ptr] = interp.m_mb.gp_regs_32[regid];
}

void VM::callbacks::sb_cb(Interpreter &interp, RegID regid, MemPtr ptr) {
  interp.m_mb.memory[ptr] =
      static_cast<uint8_t>(interp.m_mb.gp_regs_32[regid] & 0xff);
}

void VM::callbacks::shw_cb(Interpreter &interp, RegID regid, MemPtr ptr) {
  interp.m_mb.memory[ptr] =
      static_cast<uint16_t>(interp.m_mb.gp_regs_32[regid] & 0xffff);
}

void VM::callbacks::sf_cb(Interpreter &interp, RegID regid, MemPtr ptr) {
  interp.m_mb.memory[ptr] = interp.m_mb.fl_regs_32[regid];
}

void VM::callbacks::sll_cb(Interpreter &interp, RegID reg, RegID shift_by_reg,
                           RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] = interp.m_mb.gp_regs_32[reg]
                                     << interp.m_mb.gp_regs_32[shift_by_reg];
}

void VM::callbacks::srl_cb(Interpreter &interp, RegID reg, RegID shift_by_reg,
                           RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] =
      interp.m_mb.gp_regs_32[reg] >> interp.m_mb.gp_regs_32[shift_by_reg];
}

void VM::callbacks::slli_cb(Interpreter &interp, RegID reg1, uint32_t shift_by,
                            RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] = interp.m_mb.gp_regs_32[reg1] << shift_by;
}

void VM::callbacks::srli_cb(Interpreter &interp, RegID reg1, uint32_t shift_by,
                            RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] = interp.m_mb.gp_regs_32[reg1] >> shift_by;
}

void VM::callbacks::add_cb(Interpreter &interp, RegID reg1, RegID reg2,
                           RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] =
      interp.m_mb.gp_regs_32[reg1] + interp.m_mb.gp_regs_32[reg2];
}

void VM::callbacks::addi_cb(Interpreter &interp, RegID reg1, uint32_t imm_val,
                            RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] = interp.m_mb.gp_regs_32[reg1] + imm_val;
}

void VM::callbacks::sub_cb(Interpreter &interp, RegID reg1, RegID reg2,
                           RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] =
      interp.m_mb.gp_regs_32[reg1] - interp.m_mb.gp_regs_32[reg2];
}

void VM::callbacks::subi_cb(Interpreter &interp, RegID reg1, uint32_t imm_val,
                            RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] = interp.m_mb.gp_regs_32[reg1] - imm_val;
}

void VM::callbacks::mul_cb(Interpreter &interp, RegID reg1, RegID reg2,
                           RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] =
      interp.m_mb.gp_regs_32[reg1] * interp.m_mb.gp_regs_32[reg2];
}

void VM::callbacks::muli_cb(Interpreter &interp, RegID reg1, uint32_t imm_val,
                            RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] = interp.m_mb.gp_regs_32[reg1] * imm_val;
}

void VM::callbacks::div_cb(Interpreter &interp, RegID reg1, RegID reg2,
                           RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] =
      interp.m_mb.gp_regs_32[reg1] / interp.m_mb.gp_regs_32[reg2];
}

void VM::callbacks::divi_cb(Interpreter &interp, RegID reg1, uint32_t imm_val,
                            RegID dest_reg) {
  interp.m_mb.gp_regs_32[dest_reg] = interp.m_mb.gp_regs_32[reg1] / imm_val;
}

void VM::callbacks::addf_cb(Interpreter &interp, RegID reg1, RegID reg2,
                            RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] =
      interp.m_mb.fl_regs_32[reg1] + interp.m_mb.fl_regs_32[reg2];
}

void VM::callbacks::addif_cb(Interpreter &interp, RegID reg1, float imm_val,
                             RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] = interp.m_mb.fl_regs_32[reg1] + imm_val;
}

void VM::callbacks::subf_cb(Interpreter &interp, FL_RegID reg1, FL_RegID reg2,
                            FL_RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] =
      interp.m_mb.fl_regs_32[reg1] - interp.m_mb.fl_regs_32[reg2];
}

void VM::callbacks::subif_cb(Interpreter &interp, FL_RegID reg1, float imm_val,
                             FL_RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] = interp.m_mb.fl_regs_32[reg1] - imm_val;
}

void VM::callbacks::mulf_cb(Interpreter &interp, FL_RegID reg1, RegID reg2,
                            FL_RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] =
      interp.m_mb.fl_regs_32[reg1] * interp.m_mb.fl_regs_32[reg2];
}

void VM::callbacks::mulif_cb(Interpreter &interp, FL_RegID reg1, float imm_val,
                             FL_RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] = interp.m_mb.fl_regs_32[reg1] * imm_val;
}

void VM::callbacks::divf_cb(Interpreter &interp, FL_RegID reg1, FL_RegID reg2,
                            FL_RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] =
      interp.m_mb.fl_regs_32[reg1] / interp.m_mb.fl_regs_32[reg2];
}

void VM::callbacks::divif_cb(Interpreter &interp, FL_RegID reg1, float imm_val,
                             FL_RegID dest_reg) {
  interp.m_mb.fl_regs_32[dest_reg] = interp.m_mb.fl_regs_32[reg1] / imm_val;
}

void VM::callbacks::jmp_cb(Interpreter &interp, MemPtr addr) {
  check_mem_address_with_throw(addr);
  interp.m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG] = addr;
}

// NOTE: Jump zero is the same thing as jump equal because the comparison uses
// subtraction to compare two values and if the result is zero that means the
// values are equal.
void VM::callbacks::jz_cb(Interpreter &interp, MemPtr addr) {
  if (interp.m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT)) {
    jmp_cb(interp, addr);
  }
}

void VM::callbacks::jzr_cb(Interpreter &interp, RegID regid) {
  jz_cb(interp, interp.m_mb.gp_regs_32[regid]);
}

void VM::callbacks::je_cb(Interpreter &interp, MemPtr addr) {
  if (interp.m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT)) {
    jmp_cb(interp, addr);
  }
}

void VM::callbacks::jer_cb(Interpreter &interp, RegID regid) {
  je_cb(interp, interp.m_mb.gp_regs_32[regid]);
}

void VM::callbacks::jlt_cb(Interpreter &interp, MemPtr addr) {
  if (interp.m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT) ||
      interp.m_mb.check_flag(MemoryBank::SIGN_FLAG_BIT) !=
          interp.m_mb.check_flag(MemoryBank::OVERFLOW_FLAG_BIT)) {
    jmp_cb(interp, addr);
  }
}

void VM::callbacks::jltr_cb(Interpreter &interp, RegID regid) {
  jlt_cb(interp, interp.m_mb.gp_regs_32[regid]);
}

void VM::callbacks::jgt_cb(Interpreter &interp, MemPtr addr) {
  if (interp.m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT) &&
      interp.m_mb.check_flag(MemoryBank::SIGN_FLAG_BIT) ==
          interp.m_mb.check_flag(MemoryBank::OVERFLOW_FLAG_BIT)) {
    jmp_cb(interp, addr);
  }
}

void VM::callbacks::jgtr_cb(Interpreter &interp, RegID regid) {
  jgt_cb(interp, interp.m_mb.gp_regs_32[regid]);
}

void VM::callbacks::halt_cb(Interpreter &interp) { interp.stop(); }

// NOTE: In order to load jump to instruction we need to copy the instructions
// into memory first.

void VM::callbacks::jmpr_cb(Interpreter &interp, RegID regid) {
  check_mem_address_with_throw(interp.m_mb.gp_regs_32[regid]);
  interp.m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG] =
      interp.m_mb.gp_regs_32[regid];
}

// NOTE: Jump zero is the same thing as jump equal because the comparison uses
// subtraction to compare two values and if the result is zero that means the
// values are equal.

void VM::callbacks::cmp_cb(Interpreter &interp, RegID reg1, RegID reg2) {
  uint64_t res = (uint64_t)interp.m_mb.fl_regs_32[reg1] -
                 (uint64_t)interp.m_mb.fl_regs_32[reg2];

  if (res > INT_MIN || res < INT_MAX) {
    interp.m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  }

  if (res == 0.0f) {
    interp.m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
  } else if (res > 0.0f) {
    interp.m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  } else {
    interp.m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  }
}

void VM::callbacks::cmpf_cb(Interpreter &interp, RegID reg1, RegID reg2) {
  float res = 0.0f;
  try {
    res = boost::numeric::converter<float, double>::convert(
        (double)interp.m_mb.fl_regs_32[reg1] -
        (double)interp.m_mb.fl_regs_32[reg2]);
  } catch (boost::numeric::positive_overflow const &) {
    interp.m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  } catch (boost::numeric::negative_overflow const &) {
    interp.m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  }

  if (res == 0.0f) {
    interp.m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
  } else if (res > 0.0f) {
    interp.m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  } else {
    interp.m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  }
}

void check_bytes_ahead(MemoryBank::MemoryBuffer &buffer, size_t index,
                       size_t count) {
  if (index < 0 && index + count > buffer.size()) {
    throw std::runtime_error(
        (boost::format("MemoryBank::MemoryBuffer does not have %1% bytes "
                       "ahead(index: %2%).") %
         count % index)
            .str());
  }
}

RegID read_valid_regid(MemoryBank::MemoryBuffer &buffer, uint32_t &pc) {

  uint8_t rid = buffer[pc];

  if (rid < 0 && rid > MemoryBank::GP_REGS_32_COUNT) {
    throw std::runtime_error(
        (boost::format("Invalid Register ID: %1%") % rid).str());
  }
  return rid;
}

RegID read_valid_float_regid(MemoryBank::MemoryBuffer &buffer, uint32_t &pc) {
  uint8_t rid = buffer[pc];

  if (rid < 0 && rid > MemoryBank::FL_REGS_32_COUNT) {
    throw std::runtime_error(
        (boost::format("Invalid Float Register ID: %1%") % rid).str());
  }
  return rid;
}

MemPtr read_valid_mem_address(MemoryBank::MemoryBuffer &buffer, uint32_t &pc) {

  check_bytes_ahead(buffer, pc, sizeof(MemPtr));
  auto mem_ptr = *reinterpret_cast<MemPtr *>(&buffer[pc]);
  // std::cout << "MEM ADDRESS READ: " << mem_ptr << std::endl;
  return mem_ptr;
}

// FIXME: Use concepts to restrict the type to integers

float read_valid_float_immediate_val(MemoryBank::MemoryBuffer &buffer,
                                     uint32_t &pc) {
  check_bytes_ahead(buffer, pc, 4);
  auto res = *reinterpret_cast<float *>(&buffer[pc]);

  return res;
}

MemPtr check_mem_address_with_throw(MemPtr mem_ptr) {
  if (mem_ptr < 0 && mem_ptr > MemoryBank::MEMORY_SIZE) {
    throw std::runtime_error(
        (boost::format("Invalid Memory Address ID: %1%") % mem_ptr).str());
  }

  return mem_ptr;
}
