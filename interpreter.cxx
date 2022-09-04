#include "interpreter.hxx"
#include <boost/format.hpp>

using OpCodes = VirtualMachine::Instruction::OpCodes;
using RegID = VirtualMachine::RegID;
using MemPtr = VirtualMachine::MemPtr;


// NOTE: We need to check for endienness in immediate values, memory addresses etc.

RegID VirtualMachine::Interpreter::read_valid_regid(RegID rid) const
{
  if(rid < 0 && rid > 15) {
    throw std::runtime_error((boost::format("Invalid Register ID: %1%") % rid).str());
  }

  return rid;
}

RegID VirtualMachine::Interpreter::read_valid_float_regid(RegID rid) const
{
  if(rid < 0 && rid > 15) {
    throw std::runtime_error((boost::format("Invalid Float Register ID: %1%") % rid).str());
  }

  return rid;
}

void VirtualMachine::Interpreter::check_bytes_ahead(BytecodeBuffer &buffer, size_t index, size_t count) const
{
  if(index < 0 && index+count > buffer.size()) {
    throw std::runtime_error((boost::format("BytecodeBuffer does not have %1% bytes ahead(index: %2%).") % count % index).str());
  }
}

MemPtr VirtualMachine::Interpreter::read_valid_mem_address(BytecodeBuffer &buffer, size_t index) const
{

  check_bytes_ahead(buffer, index, 4);
  auto mem_ptr  = *reinterpret_cast<MemPtr*>(&buffer[index]);

  if(mem_ptr < 0 && mem_ptr > VirtualMachine::Interpreter::MemoryBank::MEMORY_SIZE) {
    throw std::runtime_error((boost::format("Invalid Memory Address ID: %1%") % mem_ptr).str());
  }

  return mem_ptr;
}

int VirtualMachine::Interpreter::read_valid_int_immediate_val(BytecodeBuffer &buffer, size_t index) const
{
  check_bytes_ahead(buffer, index, 4);
  return *reinterpret_cast<int*>(&buffer[index]);
}


float VirtualMachine::Interpreter::read_valid_float_immediate_val(BytecodeBuffer &buffer, size_t index) const
{
  check_bytes_ahead(buffer, index, 4);
  return *reinterpret_cast<float*>(&buffer[index]);
}

void VirtualMachine::Interpreter::execute(BytecodeBuffer &buffer)
{
  for(size_t i = 0; i < buffer.size(); i++) {
    switch (i) {
    case OpCodes::PUSH_STACK:{
      read_valid_regid(buffer[i++]);
      push_stack(buffer[i]);
      // 1 byte regid
      i++;
      continue;
    }
    case OpCodes::POP_STACK:{
      read_valid_regid(buffer[i++]);
      pop_stack(buffer[i]);
      // 1 byte regid
      i++;
      continue;
    }
    case OpCodes::PUSH_FLOAT_STACK:{
      read_valid_float_regid(buffer[i++]);
      push_stack(buffer[i]);
      // 1 byte regid
      i++;
      continue;
    }
    case OpCodes::POP_FLOAT_STACK:{
      read_valid_float_regid(buffer[i++]);
      pop_stack(buffer[i]);
      // 1 byte regid
      i++;
      continue;
    }
    case OpCodes::LOAD:{
      RegID rid = read_valid_regid(buffer[i++]);
      MemPtr addr = read_valid_mem_address(buffer, i++);
      load(rid, addr);
      // 1 byte regid, 4 bytes address
      i+=5;
      continue;
    }
    case OpCodes::LOAD_IMMEDIATE:{
      RegID rid = read_valid_regid(buffer[i++]);
      int imm = read_valid_int_immediate_val(buffer, i++);
      // 1 byte regid, 4 byte immediate value
      i+=5;
      continue;
    }
    case OpCodes::LOAD_FLOAT:{
      RegID rid = read_valid_regid(buffer[i++]);
      int imm = read_valid_float_immediate_val(buffer, i++);
      // 1 byte regid, 4 byte float immediate value
      i+=5;
      continue;
    }
    case OpCodes::LOAD_IMMEDIATE_FLOAT:{ continue; }

    case OpCodes::STORE_MEMORY:{ continue; }
    case OpCodes::STORE_FLOAT:{ continue; }

    case OpCodes::SHIFT_LEFT:{ continue; }
    case OpCodes::SHIFT_RIGHT:{ continue; }
    case OpCodes::SHIFT_IMMEDIATE_LEFT:{ continue; }
    case OpCodes::SHIFT_IMMEDIATE_RIGHT:{ continue; }

    case OpCodes::ADD_INT:{ continue; }
    case OpCodes::SUB_INT:{ continue; }
    case OpCodes::DIV_INT:{ continue; }
    case OpCodes::MULT_INT:{ continue; }

    case OpCodes::PLUS_IMMEDIATE_INT:{ continue; }
    case OpCodes::MINUS_IMMEDIATE_INT:{ continue; }
    case OpCodes::DIV_IMMEDIATE_INT:{ continue; }
    case OpCodes::MULT_IMMEDIATE_INT:{ continue; }

    case OpCodes::ADD_FLOAT:{ continue; }
    case OpCodes::SUB_FLOAT:{ continue; }
    case OpCodes::DIV_FLOAT:{ continue; }
    case OpCodes::MULT_FLOAT:{ continue; }

    case OpCodes::PLUS_IMMEDIATE_FLOAT:{ continue; }
    case OpCodes::MINUS_IMMEDIATE_FLOAT:{ continue; }
    case OpCodes::DIV_IMMEDIATE_FLOAT:{ continue; }
    case OpCodes::MULT_IMMEDIATE_FLOAT:{ continue; }

    case OpCodes::JUMP:{ continue; }
    case OpCodes::JUMP_ZERO:{ continue; }
    case OpCodes::JUMP_EQ:{ continue; }
    case OpCodes::JUMP_LT:{ continue; }
    case OpCodes::JUMP_GT:{ continue; }

    case OpCodes::COMPARE:{ continue; }
    case OpCodes::COMPARE_FLOAT:{ continue; }
    }
  }

}

using Interpreter = VirtualMachine::Interpreter;

void Interpreter::push_stack(RegID reg)
{
  m_mb.push_register_to_stack(reg);
}

void Interpreter::pop_stack(RegID reg)
{
  m_mb.pop_register_to_stack(reg);
}

void Interpreter::load(RegID reg, MemPtr ptr) {}
void Interpreter::store(RegID reg, MemPtr ptr) {}
void Interpreter::load_immediate(uint32_t val, RegID reg) {}
void Interpreter::store_float(RegID reg, MemPtr ptr) {}
void Interpreter::load_float(RegID reg, MemPtr ptr) {}
void Interpreter::load_immediate_float(float val, RegID reg) {}

void Interpreter::shift_left(RegID reg1, RegID shit_by_reg2, RegID dest_reg) {}
void Interpreter::shift_right(RegID reg1, RegID shit_by_reg2, RegID dest_reg) {}
void Interpreter::shift_left_immediate(RegID reg1, uint32_t shift_by, RegID dest_reg) {}
void Interpreter::shift_right_immediate(RegID reg1, uint32_t shift_by, RegID dest_reg) {}

void Interpreter::add(RegID reg1, RegID reg2, RegID dest_reg) {}
void Interpreter::sub(RegID reg1, RegID reg2, RegID dest_reg) {}
void Interpreter::mult(RegID reg1, RegID reg2, RegID dest_reg) {}
void Interpreter::div(RegID reg1, RegID reg2, RegID dest_reg) {}

void Interpreter::add_float(RegID reg1, RegID reg2, RegID dest_reg) {}
void Interpreter::sub_float(RegID reg1, RegID reg2, RegID dest_reg) {}
void Interpreter::mult_float(RegID reg1, RegID reg2, RegID dest_reg) {}
void Interpreter::div_float(RegID reg1, RegID reg2, RegID dest_reg) {}

void Interpreter::jump(RegID reg) {}
void Interpreter::jump_zero(RegID reg) {}
void Interpreter::jump_eq(RegID reg) {}
void Interpreter::jump_lt(RegID reg) {}
void Interpreter::jump_gt(RegID reg) {}

void Interpreter::cmp(RegID reg1, RegID reg2) {}
void Interpreter::cmp_float(RegID reg1, RegID reg2) {}
