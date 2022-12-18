#include "interpreter.hxx"
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

void Interpreter::push_stack(RegID regid) {
  m_mb.push_register_to_stack(regid);
}

void Interpreter::pop_stack(RegID regid) { m_mb.pop_register_to_stack(regid); }

void Interpreter::load(RegID regid, MemPtr ptr) {
  m_mb.gp_regs_32[regid] = m_mb.memory[ptr];
}

void Interpreter::load_immediate(RegID regid, uint32_t val) {
  m_mb.gp_regs_32[regid] = val;
}

void Interpreter::load_float(RegID regid, MemPtr ptr) {
  m_mb.fl_regs_32[regid] = m_mb.memory[ptr];
}

void Interpreter::load_immediate_float(RegID regid, float val) {
  m_mb.fl_regs_32[regid] = val;
}

void Interpreter::store(RegID regid, MemPtr ptr) {
  m_mb.memory[ptr] = m_mb.gp_regs_32[regid];
}

void Interpreter::store_float(RegID regid, MemPtr ptr) {
  m_mb.memory[ptr] = m_mb.fl_regs_32[regid];
}

void Interpreter::shift_left(RegID reg, RegID shift_by_reg, RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg]
                              << m_mb.gp_regs_32[shift_by_reg];
}

void Interpreter::shift_right(RegID reg, RegID shift_by_reg, RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] =
      m_mb.gp_regs_32[reg] >> m_mb.gp_regs_32[shift_by_reg];
}

void Interpreter::shift_left_immediate(RegID reg1, uint32_t shift_by,
                                       RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] << shift_by;
}

void Interpreter::shift_right_immediate(RegID reg1, uint32_t shift_by,
                                        RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] >> shift_by;
}

void Interpreter::add(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] + m_mb.gp_regs_32[reg2];
}

void Interpreter::sub(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] - m_mb.gp_regs_32[reg2];
}

void Interpreter::mult(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] * m_mb.gp_regs_32[reg2];
}

void Interpreter::div(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] / m_mb.gp_regs_32[reg2];
}

void Interpreter::add_float(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] + m_mb.fl_regs_32[reg2];
}

void Interpreter::sub_float(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] - m_mb.fl_regs_32[reg2];
}

void Interpreter::mult_float(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] * m_mb.fl_regs_32[reg2];
}

void Interpreter::div_float(RegID reg1, RegID reg2, RegID dest_reg) {
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] / m_mb.fl_regs_32[reg2];
}

// NOTE: In order to load jump to instruction we need to copy the instructions
// into memory first.

void Interpreter::jump(RegID regid) {
  check_mem_address_with_throw(m_mb.gp_regs_32[regid]);
  m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG] = m_mb.gp_regs_32[regid];
}

// NOTE: Jump zero is the same thing as jump equal because the comparison uses
// subtraction to compare two values and if the result is zero that means the
// values are equal.
void Interpreter::jump_zero(RegID regid) {
  check_mem_address_with_throw(m_mb.gp_regs_32[regid]);
  if (m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT)) {
    jump(regid);
  }
}

void Interpreter::jump_lt(RegID reg) {
  check_mem_address_with_throw(m_mb.gp_regs_32[reg]);
  if (m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT) ||
      m_mb.check_flag(MemoryBank::SIGN_FLAG_BIT) !=
          m_mb.check_flag(MemoryBank::OVERFLOW_FLAG_BIT)) {
    jump(reg);
  }
}

void Interpreter::jump_gt(RegID reg) {
  check_mem_address_with_throw(m_mb.gp_regs_32[reg]);
  if (m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT) &&
      m_mb.check_flag(MemoryBank::SIGN_FLAG_BIT) ==
          m_mb.check_flag(MemoryBank::OVERFLOW_FLAG_BIT)) {
    jump(reg);
  }
}

void Interpreter::cmp(RegID reg1, RegID reg2) {
  uint64_t res =
      (uint64_t)m_mb.fl_regs_32[reg1] - (uint64_t)m_mb.fl_regs_32[reg2];

  if (res > INT_MIN || res < INT_MAX) {
    m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  }

  if (res == 0.0f) {
    m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
  } else if (res > 0.0f) {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  } else {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  }
}

void Interpreter::cmp_float(RegID reg1, RegID reg2) {
  float res = 0.0f;
  try {
    res = boost::numeric::converter<float, double>::convert(
        (double)m_mb.fl_regs_32[reg1] - (double)m_mb.fl_regs_32[reg2]);
  } catch (boost::numeric::positive_overflow const &) {
    m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  } catch (boost::numeric::negative_overflow const &) {
    m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  }

  if (res == 0.0f) {
    m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
  } else if (res > 0.0f) {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  } else {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
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

template <typename IntType> IntType vm_to_host_number(IntType n) { return n; }

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

// NOTE: This is an auxilary function to make the code more readable.
template <typename Type> Type *looking_at_cast(uint8_t *ptr) {
  return reinterpret_cast<Type *>(ptr);
}

// FIXME: Use concepts to restrict the type to integers
template <typename IntType>
IntType read_valid_int_immediate_val(MemoryBank::MemoryBuffer &buffer,
                                     uint32_t &pc) {

  check_bytes_ahead(buffer, pc, sizeof(IntType));
  int32_t res =
      vm_to_host_number<IntType>(*looking_at_cast<IntType>(&buffer[pc]));
  return res;
}

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
