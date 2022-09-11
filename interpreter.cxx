#include "interpreter.hxx"
#include <boost/format.hpp>
#include <climits>
#include <boost/numeric/conversion/converter.hpp>
#include <boost/limits.hpp>

using OpCodes = VirtualMachine::Instruction::OpCodes;
using RegID = VirtualMachine::RegID;
using MemPtr = VirtualMachine::MemPtr;


// NOTE: We need to check for endienness in immediate values, memory addresses etc.

RegID VirtualMachine::Interpreter::read_valid_regid(MemoryBuffer& buffer, uint32_t& pc) const
{

  uint8_t rid = buffer[pc];

  if(rid < 0 && rid > 15) {
    throw std::runtime_error((boost::format("Invalid Register ID: %1%") % rid).str());
  }

  pc+=1;
  return rid;
}

RegID VirtualMachine::Interpreter::read_valid_float_regid(MemoryBuffer& buffer, uint32_t& pc) const
{
  uint8_t rid = buffer[pc];

  if(rid < 0 && rid > 15) {
    throw std::runtime_error((boost::format("Invalid Float Register ID: %1%") % rid).str());
  }

  pc+=1;
  return rid;
}


MemPtr VirtualMachine::Interpreter::read_valid_mem_address(MemoryBuffer &buffer, uint32_t& pc) const
{

  check_bytes_ahead(buffer, pc, 4);
  auto mem_ptr  = *reinterpret_cast<MemPtr*>(&buffer[pc]);

  pc+=4;
  return mem_ptr;
}

int VirtualMachine::Interpreter::read_valid_int_immediate_val(MemoryBuffer &buffer, uint32_t& pc) const
{
  check_bytes_ahead(buffer, pc, 4);
  auto res = *reinterpret_cast<int*>(&buffer[pc]);

  pc+=4;
  return res;
}

float VirtualMachine::Interpreter::read_valid_float_immediate_val(MemoryBuffer &buffer, uint32_t& pc) const
{
  check_bytes_ahead(buffer, pc, 4);
  auto res = *reinterpret_cast<float*>(&buffer[pc]);

  pc+=4;
  return res;
}

void VirtualMachine::Interpreter::check_bytes_ahead(MemoryBuffer &buffer, size_t index, size_t count) const
{
  if(index < 0 && index+count > buffer.size()) {
    throw std::runtime_error((boost::format("MemoryBuffer does not have %1% bytes ahead(index: %2%).") % count % index).str());
  }
}

MemPtr VirtualMachine::Interpreter::check_mem_address_with_throw(MemPtr mem_ptr) const
{
  if(mem_ptr < 0 && mem_ptr > VirtualMachine::Interpreter::MemoryBank::MEMORY_SIZE) {
    throw std::runtime_error((boost::format("Invalid Memory Address ID: %1%") % mem_ptr).str());
  }

  return mem_ptr;
}

void VirtualMachine::Interpreter::load_program(BytecodeBuffer &buffer)
{
  if(buffer.size() > m_mb.memory.size()) {
    throw std::runtime_error(( boost::format("Program too large for VM memory!(size: %1%)") % buffer.size() ).str());
  }

  // Load the program to address 0
  std::copy(begin(buffer), end(buffer), begin(m_mb.memory));
}


void VirtualMachine::Interpreter::run()
{
  // NOTE: We can copy all the instruction into a local buffer and exit, as the interpreter can run on
  // a seperate thread.
  //
  // NOTE: Program can be loaded to the address 0 in VM memory.
  // NOTE: When we stamble upon a invalid instruction we can raise an interrupt to handle the error.
  // NOTE: Maybe we want a bootloader in order to load a kernel as well as some standard boot process.
  // FIXME: The memory buffer is used as storage for the program and we are accessing the memory
  // to retrieve instructions, we're not using a bytecode buffers.

  auto& buffer = m_mb.memory;
  auto& pc = m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG];

  for(pc = 0; pc < m_mb.memory.size(); pc++) {
    switch (buffer[pc]) {
    case OpCodes::PUSH_STACK:{
      RegID rid = read_valid_regid(buffer, pc);
      push_stack(rid);
      continue;
    }
    case OpCodes::POP_STACK:{
      RegID rid = read_valid_regid(buffer, pc);
      pop_stack(rid);
      continue;
    }
    case OpCodes::PUSH_FLOAT_STACK:{
      RegID rid = read_valid_float_regid(buffer, pc);
      push_stack(rid);
      pc += 1;
      continue;
    }
    case OpCodes::POP_FLOAT_STACK:{
      RegID rid = read_valid_float_regid(buffer, pc);
      pop_stack(rid);
      pc += 1;
      continue;
    }
    case OpCodes::LOAD:{
      RegID rid = read_valid_regid(buffer, pc);
      MemPtr addr = read_valid_mem_address(buffer, pc);
      check_mem_address_with_throw(addr);
      load(rid, addr);
      continue;
    }
    case OpCodes::LOAD_IMMEDIATE:{
      RegID rid = read_valid_regid(buffer, pc);
      int imm = read_valid_int_immediate_val(buffer, pc);
      continue;
    }
    case OpCodes::LOAD_FLOAT:{
      RegID rid = read_valid_regid(buffer, pc);
      int imm = read_valid_float_immediate_val(buffer, pc);
      continue;
    }
    case OpCodes::LOAD_IMMEDIATE_FLOAT: {
      RegID rid = read_valid_regid(buffer, pc);
      int imm = read_valid_float_immediate_val(buffer, pc);
      continue;
    }
    case OpCodes::STORE:{
      MemPtr addr = read_valid_mem_address(buffer, pc);
      RegID rid = read_valid_regid(buffer, pc);
      check_mem_address_with_throw(addr);
      store(rid, addr);
      continue;
    }
    case OpCodes::STORE_FLOAT:{
      MemPtr addr = read_valid_mem_address(buffer, pc);
      RegID rid = read_valid_regid(buffer, pc);
      check_mem_address_with_throw(addr);
      store_float(rid, addr);
      continue;
    }
    case OpCodes::SHIFT_LEFT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      shift_left(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::SHIFT_RIGHT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      shift_right(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::SHIFT_IMMEDIATE_LEFT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_int_immediate_val(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      shift_right(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::SHIFT_IMMEDIATE_RIGHT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_int_immediate_val(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      shift_right(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::ADD_INT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      add(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::SUB_INT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      sub(rid1, rid2, rid3);
     continue;
    }
    case OpCodes::DIV_INT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      div(rid1, rid2, rid3);
     continue;
    }
    case OpCodes::MULT_INT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      mult(rid1, rid2, rid3);
      continue;
    }

    // case OpCodes::PLUS_IMMEDIATE_INT:{continue;}
    // case OpCodes::MINUS_IMMEDIATE_INT:{continue;}
    // case OpCodes::DIV_IMMEDIATE_INT:{continue;}

    case OpCodes::MULT_IMMEDIATE_INT:{
      continue;
    }
    case OpCodes::ADD_FLOAT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      add_float(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::SUB_FLOAT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      sub_float(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::DIV_FLOAT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      div_float(rid1, rid2, rid3);
      continue;
    }
    case OpCodes::MULT_FLOAT:{ continue;
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      RegID rid3 = read_valid_regid(buffer, pc);
      mult_float(rid1, rid2, rid3);
    }

    // case OpCodes::PLUS_IMMEDIATE_FLOAT:{ continue; }
    // case OpCodes::MINUS_IMMEDIATE_FLOAT:{ continue; }
    // case OpCodes::DIV_IMMEDIATE_FLOAT:{ continue; }
    // case OpCodes::MULT_IMMEDIATE_FLOAT:{ continue; }

    case OpCodes::JUMP:{
      RegID rid = read_valid_regid(buffer, pc);
      jump(rid);
      continue;
    }
    case OpCodes::JUMP_ZERO:{
      RegID rid = read_valid_regid(buffer, pc);
      jump_zero(rid);
      continue;
    }
    case OpCodes::JUMP_EQ:{
      RegID rid = read_valid_regid(buffer, pc);
      jump_zero(rid);
      continue;
    }
    case OpCodes::JUMP_LT:{
      RegID rid = read_valid_regid(buffer, pc);
      jump_lt(rid);
      continue;
    }
    case OpCodes::JUMP_GT:{
      RegID rid = read_valid_regid(buffer, pc);
      jump_gt(rid);
      continue;
    }
    case OpCodes::COMPARE:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      cmp(rid1, rid2);
      continue;
    }
    case OpCodes::COMPARE_FLOAT:{
      RegID rid1 = read_valid_regid(buffer, pc);
      RegID rid2 = read_valid_regid(buffer, pc);
      cmp_float(rid1, rid2);
      continue;
    }
    default:
      throw std::runtime_error((boost::format("Invalid instruction (OPCODE: %1%, PC: %2%)") % buffer[pc] % pc).str());
    }
  }
}

using Interpreter = VirtualMachine::Interpreter;


// NOTE: Don't implement everything in the MemoryBank because than we unnecessarily have bigger call stack.
// Just access private members variables of the MemoryBank from here.
// NOTE: It might be more efficient to verify memory pointers during decoding in the excecute method.
// NOTE: Instead of interpreting the bytecode directly we can use JIT (Just in Time) compilation
// to native machine code.
// NOTE: Mathematical operations are done in order the values are supplied in.
// TODO: The arithmetic instruction need to raise flags!
// TODO: Go through all the instructions and verify they are implemented correctly.

void Interpreter::push_stack(RegID regid)
{
  m_mb.push_register_to_stack(regid);
}

void Interpreter::pop_stack(RegID regid)
{
  m_mb.pop_register_to_stack(regid);
}

void Interpreter::load(RegID regid, MemPtr ptr)
{
  m_mb.gp_regs_32[regid] = m_mb.memory[ptr];
}

void Interpreter::load_immediate(uint32_t val, RegID regid)
{
  m_mb.gp_regs_32[regid] = val;
}

void Interpreter::load_float(RegID regid, MemPtr ptr)
{
  m_mb.fl_regs_32[regid] = m_mb.memory[ptr];
}

void Interpreter::load_immediate_float(float val, RegID regid)
{
  m_mb.fl_regs_32[regid] = val;
}

void Interpreter::store(RegID regid, MemPtr ptr)
{
  m_mb.memory[ptr] = m_mb.gp_regs_32[regid];
}

void Interpreter::store_float(RegID regid, MemPtr ptr)
{
  m_mb.memory[ptr] = m_mb.fl_regs_32[regid];
}

void Interpreter::shift_left(RegID reg, RegID shift_by_reg, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg] <<  m_mb.gp_regs_32[shift_by_reg];
}

void Interpreter::shift_right(RegID reg, RegID shift_by_reg, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg] >>  m_mb.gp_regs_32[shift_by_reg];
}

void Interpreter::shift_left_immediate(RegID reg1, uint32_t shift_by, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] << shift_by;
}

void Interpreter::shift_right_immediate(RegID reg1, uint32_t shift_by, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] >> shift_by;
}

void Interpreter::add(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] + m_mb.gp_regs_32[reg2];
}

void Interpreter::sub(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] - m_mb.gp_regs_32[reg2];
}

void Interpreter::mult(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] * m_mb.gp_regs_32[reg2];
}

void Interpreter::div(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.gp_regs_32[dest_reg] = m_mb.gp_regs_32[reg1] / m_mb.gp_regs_32[reg2];
}

void Interpreter::add_float(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] + m_mb.fl_regs_32[reg2];
}

void Interpreter::sub_float(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] - m_mb.fl_regs_32[reg2];
}

void Interpreter::mult_float(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] * m_mb.fl_regs_32[reg2];
}

void Interpreter::div_float(RegID reg1, RegID reg2, RegID dest_reg)
{
  m_mb.fl_regs_32[dest_reg] = m_mb.fl_regs_32[reg1] / m_mb.fl_regs_32[reg2];
}

// NOTE: In order to load jump to instruction we need to copy the instructions into memory first.

void Interpreter::jump(RegID regid)
{
  check_mem_address_with_throw(m_mb.gp_regs_32[regid]);
  m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG] = m_mb.gp_regs_32[regid];
}

// NOTE: Jump zero is the same thing as jump equal because the comparison uses subtraction to
// compare two values and if the result is zero that means the values are equal.
void Interpreter::jump_zero(RegID regid)
{
  check_mem_address_with_throw(m_mb.gp_regs_32[regid]);
  if(m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT)) {
    jump(regid);
  }
}

void Interpreter::jump_lt(RegID reg)
{
  check_mem_address_with_throw(m_mb.gp_regs_32[reg]);
  if(m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT) || m_mb.check_flag(MemoryBank::SIGN_FLAG_BIT) != m_mb.check_flag(MemoryBank::OVERFLOW_FLAG_BIT)) {
    jump(reg);
  }
}

void Interpreter::jump_gt(RegID reg)
{
  check_mem_address_with_throw(m_mb.gp_regs_32[reg]);
  if(m_mb.check_flag(MemoryBank::ZERO_FLAG_BIT) && m_mb.check_flag(MemoryBank::SIGN_FLAG_BIT) == m_mb.check_flag(MemoryBank::OVERFLOW_FLAG_BIT)) {
    jump(reg);
  }
}

void Interpreter::cmp(RegID reg1, RegID reg2)
{
  uint64_t res = (uint64_t)m_mb.fl_regs_32[reg1] - (uint64_t)m_mb.fl_regs_32[reg2];

  if(res > INT_MIN || res < INT_MAX) {
    m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  }

  if(res == 0.0f) {
    m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
  }
  else if (res > 0.0f) {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  }
  else {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  }
}

void Interpreter::cmp_float(RegID reg1, RegID reg2)
{
  float res = 0.0f;
  try {
    res = boost::numeric::converter<float, double>::convert((double)m_mb.fl_regs_32[reg1] - (double)m_mb.fl_regs_32[reg2]);
  } catch ( boost::numeric::positive_overflow const& ) {
    m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  } catch ( boost::numeric::negative_overflow const& ) {
    m_mb.set_flag(MemoryBank::OVERFLOW_FLAG_BIT);
  }

  if(res == 0.0f) {
    m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
  }
  else if (res > 0.0f) {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  }
  else {
    m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  }
}
