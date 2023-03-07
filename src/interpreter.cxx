#include "interpreter.hxx"
#include "instructions.hxx"
#include <boost/format.hpp>
#include <boost/limits.hpp>
#include <boost/numeric/conversion/converter.hpp>
#include <climits>
#include <cmath>

#define DEBUG_EXTRA_INFO

#define GP_REG(reg) interp.m_mb.gp_regs_32[reg]
#define GP_REG_INFO(reg)                                                       \
  "(R" << reg << " = " << interp.m_mb.gp_regs_32[reg] << ")"

#define FL_REG(reg) interp.m_mb.fl_regs_32[reg]
#define FL_REG_INFO(reg)                                                       \
  "(R" << reg << " = " << interp.m_mb.fl_regs_32[reg] << ")"

#define PC_REG interp.m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG]

#define VM_MEMORY(addr) interp.m_mb.memory[addr]

#define CHECK_FLAG(flag) interp.m_mb.check_flag(MemoryBank::flag)

using OpCodes = VM::OpCodes;

// NOTE: We need to deal with endienness in immediate values, memory addresses
// etc.

// FIXME: This function does nothing since it was writen on x86 system.
// On big-endian systems it needs to switch the bytes around to match the
// systems endianness since the vm stores numbers in little-endian format.

void Interpreter::load_program(BytecodeBuffer &buffer) {
#ifdef DEBUG_EXTRA_INFO
  std::cout << "Loading program!\n";
#endif
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
  auto &interp = *this;
  auto &pc = GP_REG(MemoryBank::PROGRAM_COUNTER_REG);
  auto &mem = m_mb.memory;

  try {
    while (pc < mem.size()) {
      VM::run_next_instruction(*this);
    }
  } catch (std::runtime_error re) {
    std::cerr << "An fatal error has occured during runtime of the "
                 "interpreter.\nRUNTIME_ERROR: "
              << re.what() << std::endl;
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
//

template <uint8_t T> using PL = VM::parameters::ParameterList<T>;
using OP = VM::OpCodes;

void VM::callbacks::invalid_cb(Interpreter &interp, const PL<OP::INVALID> &) {
  throw std::runtime_error("Invalid instruction encountered!");
}
void VM::callbacks::nop_cb(Interpreter &interp, const PL<OP::NOP> &) {}

void VM::callbacks::push_cb(Interpreter &interp, const PL<OP::PUSH_STACK> &p) {
  interp.m_mb.push_register_to_stack(p.source);
}

void VM::callbacks::pushf_cb(Interpreter &interp,
                             const PL<OP::PUSH_FLOAT_STACK> &p) {
  interp.m_mb.push_float_register_to_stack(p.source);
}

void VM::callbacks::pop_cb(Interpreter &interp, const PL<OP::POP_STACK> &p) {
  interp.m_mb.pop_register_from_stack(p.destination);
}

void VM::callbacks::popf_cb(Interpreter &interp,
                            const PL<OP::POP_FLOAT_STACK> &p) {
  interp.m_mb.pop_float_register_from_stack(p.destination);
}

void VM::callbacks::ld_cb(Interpreter &interp, const PL<OP::LOAD> &p) {
  GP_REG(p.destination) = *reinterpret_cast<uint32_t *>(&VM_MEMORY(p.source));
}

void VM::callbacks::lb_cb(Interpreter &interp, const PL<OP::LOAD_BYTE> &p) {
  GP_REG(p.destination) = *reinterpret_cast<uint8_t *>(&VM_MEMORY(p.source));
}

void VM::callbacks::lhw_cb(Interpreter &interp,
                           const PL<OP::LOAD_HALF_WORD> &p) {
  GP_REG(p.destination) = *reinterpret_cast<uint16_t *>(&VM_MEMORY(p.source));
}

void VM::callbacks::ldi_cb(Interpreter &interp,
                           const PL<OP::LOAD_IMMEDIATE> &p) {
#ifdef DEBUG_EXTRA_INFO
  std::clog << "Loading immediate value (" << p.immediate_value
            << ") into register R" << (int)p.destination << std::endl;
#endif
  GP_REG(p.destination) = p.immediate_value;
}

void VM::callbacks::lbi_cb(Interpreter &interp,
                           const PL<OP::LOAD_BYTE_IMMEDIATE> &p) {
  GP_REG(p.destination) = p.immediate_value;
}

void VM::callbacks::lhwi_cb(Interpreter &interp,
                            const PL<OP::LOAD_HALF_WORD_IMMEDIATE> &p) {
  GP_REG(p.destination) = p.immediate_value;
}

void VM::callbacks::lf_cb(Interpreter &interp, const PL<OP::LOAD_FLOAT> &p) {
  FL_REG(p.destination) = VM_MEMORY(p.source);
}

void VM::callbacks::lfi_cb(Interpreter &interp,
                           const PL<OP::LOAD_FLOAT_IMMEDIATE> &p) {
  FL_REG(p.destination) = p.immediate_value;
}

void VM::callbacks::st_cb(Interpreter &interp, const PL<OP::STORE> &p) {
  GP_REG(p.destination) = GP_REG(p.source);
}

void VM::callbacks::sb_cb(Interpreter &interp, const PL<OP::STORE_BYTE> &p) {
  VM_MEMORY(p.destination) = static_cast<uint8_t>(GP_REG(p.source) & 0xff);
}

void VM::callbacks::shw_cb(Interpreter &interp,
                           const PL<OP::STORE_HALF_WORD> &p) {
  VM_MEMORY(p.destination) = static_cast<uint16_t>(GP_REG(p.source) & 0xffff);
}

void VM::callbacks::sf_cb(Interpreter &interp, const PL<OP::STORE_FLOAT> &p) {
  VM_MEMORY(p.destination) = FL_REG(p.source);
}

void VM::callbacks::sll_cb(Interpreter &interp, const PL<OP::SHIFT_LEFT> &p) {
  GP_REG(p.destination) = GP_REG(p.source) << GP_REG(p.shift_by);
}

void VM::callbacks::srl_cb(Interpreter &interp, const PL<OP::SHIFT_RIGHT> &p) {
  GP_REG(p.destination) = GP_REG(p.source) >> GP_REG(p.shift_by);
}

void VM::callbacks::slli_cb(Interpreter &interp,
                            const PL<OP::SHIFT_IMMEDIATE_LEFT> &p) {
  GP_REG(p.destination) = GP_REG(p.source) << p.shift_by;
}

void VM::callbacks::srli_cb(Interpreter &interp,
                            const PL<OP::SHIFT_IMMEDIATE_RIGHT> &p) {
  GP_REG(p.destination) = GP_REG(p.source) >> p.shift_by;
}

void VM::callbacks::add_cb(Interpreter &interp, const PL<OP::ADD_INT> &p) {
  GP_REG(p.destination) = GP_REG(p.source1) + GP_REG(p.source2);
}

void VM::callbacks::addi_cb(Interpreter &interp,
                            const PL<OP::ADD_INT_IMMEDIATE> &p) {
  GP_REG(p.destination) = GP_REG(p.source) + p.immediate_value;
}

void VM::callbacks::sub_cb(Interpreter &interp, const PL<OP::SUB_INT> &p) {
  GP_REG(p.destination) = GP_REG(p.source1) - GP_REG(p.source2);
}

void VM::callbacks::subi_cb(Interpreter &interp,
                            const PL<OP::SUB_INT_IMMEDIATE> &p) {
  GP_REG(p.destination) = GP_REG(p.source) - p.immediate_value;
}

void VM::callbacks::mul_cb(Interpreter &interp, const PL<OP::MULT_INT> &p) {
  GP_REG(p.destination) = GP_REG(p.source1) * GP_REG(p.source2);
}

void VM::callbacks::muli_cb(Interpreter &interp,
                            const PL<OP::MULT_INT_IMMEDIATE> &p) {
  GP_REG(p.destination) = GP_REG(p.source) * p.immediate_value;
}

void VM::callbacks::div_cb(Interpreter &interp, const PL<OP::DIV_INT> &p) {
  GP_REG(p.destination) = GP_REG(p.source1) / GP_REG(p.source2);
}

void VM::callbacks::divi_cb(Interpreter &interp,
                            const PL<OP::DIV_INT_IMMEDIATE> &p) {
  GP_REG(p.destination) = GP_REG(p.source) / p.immediate_value;
}

void VM::callbacks::addf_cb(Interpreter &interp, const PL<OP::ADD_FLOAT> &p) {
  FL_REG(p.destination) = FL_REG(p.source1) + FL_REG(p.source2);
}

void VM::callbacks::addif_cb(Interpreter &interp,
                             const PL<OP::ADD_FLOAT_IMMEDIATE> &p) {
  FL_REG(p.destination) = FL_REG(p.source) + p.immediate_value;
}

void VM::callbacks::subf_cb(Interpreter &interp, const PL<OP::SUB_FLOAT> &p) {
  FL_REG(p.destination) = FL_REG(p.source1) - FL_REG(p.source2);
}

void VM::callbacks::subif_cb(Interpreter &interp,
                             const PL<OP::SUB_FLOAT_IMMEDIATE> &p) {
  FL_REG(p.destination) = FL_REG(p.source) - p.immediate_value;
}

void VM::callbacks::mulf_cb(Interpreter &interp, const PL<OP::MULT_FLOAT> &p) {
  FL_REG(p.destination) = FL_REG(p.source1) * FL_REG(p.source2);
}

void VM::callbacks::mulif_cb(Interpreter &interp,
                             const PL<OP::MULT_FLOAT_IMMEDIATE> &p) {
  FL_REG(p.destination) = FL_REG(p.source) * p.immediate_value;
}

void VM::callbacks::divf_cb(Interpreter &interp, const PL<OP::DIV_FLOAT> &p) {
#ifdef DEBUG_EXTRA_INFO
  std::clog << "Dividing two float registers " << FL_REG_INFO(p.source1)
            << " and " << FL_REG_INFO(p.source2) << "\n";
#endif
  FL_REG(p.destination) = FL_REG(p.source1) / FL_REG(p.source2);
}

void VM::callbacks::divif_cb(Interpreter &interp,
                             const PL<OP::DIV_FLOAT_IMMEDIATE> &p) {
#ifdef DEBUG_EXTRA_INFO
  std::clog << "Dividing float register with float immediate (R" << p.source
            << " = " << FL_REG(p.source) << ") and " << p.immediate_value
            << "\n.";
  std::clog << "Destination register R" << p.destination << ".\n";
#endif
  FL_REG(p.destination) = FL_REG(p.source) / p.immediate_value;
}

void VM::callbacks::jmp_cb(Interpreter &interp, const PL<OP::JUMP> &p) {
  check_mem_address_with_throw(p.jump_address);
#ifdef DEBUG_EXTRA_INFO
  std::clog << "Jumping to immediate value address (" << p.jump_address
            << ")\n";
#endif
  PC_REG = p.jump_address;
}

// NOTE: Jump zero is the same thing as jump equal because the comparison uses
// subtraction to compare two values and if the result is zero that means the
// values are equal.
void VM::callbacks::jz_cb(Interpreter &interp, const PL<OP::JUMP_ZERO> &p) {
  if (CHECK_FLAG(ZERO_FLAG_BIT)) {
    jmp_cb(interp, {p.jump_address});
  }
}

void VM::callbacks::jzr_cb(Interpreter &interp,
                           const PL<OP::JUMP_REGISTER_ZERO> &p) {
  jz_cb(interp, {GP_REG(p.jump_register)});
}

void VM::callbacks::je_cb(Interpreter &interp, const PL<OP::JUMP_EQUAL> &p) {
  if (CHECK_FLAG(ZERO_FLAG_BIT)) {
    jmp_cb(interp, {p.jump_address});
  }
}

void VM::callbacks::jer_cb(Interpreter &interp,
                           const PL<OP::JUMP_REGISTER_EQUAL> &p) {
  je_cb(interp, {GP_REG(p.jump_register)});
}

void VM::callbacks::jlt_cb(Interpreter &interp,
                           const PL<OP::JUMP_LESS_THAN> &p) {
  if (CHECK_FLAG(ZERO_FLAG_BIT) ||
      CHECK_FLAG(SIGN_FLAG_BIT) != CHECK_FLAG(OVERFLOW_FLAG_BIT)) {
    jmp_cb(interp, {p.jump_address});
  }
}

void VM::callbacks::jltr_cb(Interpreter &interp,
                            const PL<OP::JUMP_REGISTER_LESS_THAN> &p) {
  jlt_cb(interp, {GP_REG(p.jump_register)});
}

void VM::callbacks::jgt_cb(Interpreter &interp,
                           const PL<OP::JUMP_GREATER_THAN> &p) {
  if (CHECK_FLAG(ZERO_FLAG_BIT) &&
      CHECK_FLAG(SIGN_FLAG_BIT) == CHECK_FLAG(OVERFLOW_FLAG_BIT)) {
    jmp_cb(interp, {p.jump_address});
  }
}

void VM::callbacks::jgtr_cb(Interpreter &interp,
                            const PL<OP::JUMP_REGISTER_GREATER_THAN> &p) {
  jgt_cb(interp, {GP_REG(p.jump_register)});
}

// NOTE: In order to load jump to instruction we need to copy the instructions
// into memory first.

void VM::callbacks::jmpr_cb(Interpreter &interp,
                            const PL<OP::JUMP_REGISTER> &p) {
  check_mem_address_with_throw(GP_REG(p.jump_register));
#ifdef DEBUG_EXTRA_INFO
  std::clog << "Jumping to address stored in register (R" << p.jump_register
            << " = " << GP_REG(p.jump_register) << ").\n";
#endif
  PC_REG = GP_REG(p.jump_register);
}

void VM::callbacks::halt_cb(Interpreter &interp, const PL<OP::HALT> &) {
#ifdef DEBUG_EXTRA_INFO
  std::clog << "Halting the machine.\n";
#endif
  interp.stop();
}

// NOTE: Jump zero is the same thing as jump equal because the comparison uses
// subtraction to compare two values and if the result is zero that means the
// values are equal.

void VM::callbacks::cmp_cb(Interpreter &interp, const PL<OP::COMPARE> &p) {

  // FUN_FACT: When I first implemented this, instead of using the comparison
  // operator I tried to implement it in terms of how CPU's compare numbers in
  // hardware. The only thing that insterest me how I got to that idea in the
  // first place.

#ifdef DEBUG_EXTRA_INFO
  std::clog << "Comparing registers "
            << "R" << (int)p.register1 << " and "
            << "R" << (int)p.register2 << std::endl;
#endif
  auto v1 = GP_REG(p.register1);
  auto v2 = GP_REG(p.register2);

  // NOTE: We are using signed integer, so how the hell is it suppose to be less
  // than zero. See this is why you stop and think.

  if (v1 == v2) {
#ifdef DEBUG_EXTRA_INFO
    std::clog << v1 << " = " << v2 << std::endl;
#endif
    // Set zero flag and unset sign flag
    interp.m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  } else if (v1 > v2) {
#ifdef DEBUG_EXTRA_INFO
    std::clog << v1 << " > " << v2 << std::endl;
#endif
    // Unset zero flag and set sign flag
    interp.m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  } else {
#ifdef DEBUG_EXTRA_INFO
    std::clog << v1 << " < " << v2 << std::endl;
#endif
    // Set unset flag and unset sign flag
    interp.m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  }
}

void VM::callbacks::cmpf_cb(Interpreter &interp,
                            const PL<OP::COMPARE_FLOAT> &p) {
  float v1 = FL_REG(p.register1);
  float v2 = FL_REG(p.register2);

#ifdef DEBUG_EXTRA_INFO
  std::clog << "Coparing two float registers R" << p.register1 << "and R"
            << p.register2 << "\n";
#endif

  if (v1 == v2) {
#ifdef DEBUG_EXTRA_INFO
    std::clog << v1 << " = " << v2 << "\n";
#endif
    interp.m_mb.set_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.unset_flag(MemoryBank::SIGN_FLAG_BIT);
  } else if (v1 > v2) {
#ifdef DEBUG_EXTRA_INFO
    std::clog << v1 << " > " << v2 << "\n";
#endif
    interp.m_mb.unset_flag(MemoryBank::ZERO_FLAG_BIT);
    interp.m_mb.set_flag(MemoryBank::SIGN_FLAG_BIT);
  } else {
#ifdef DEBUG_EXTRA_INFO
    std::clog << v1 << " < " << v2 << "\n";
#endif
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
#ifdef DEBUG_EXTRA_INFO
  std::clog << "Reading Memory Address: " << mem_ptr << std::endl;
#endif
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
