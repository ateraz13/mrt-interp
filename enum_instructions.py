import os
import json
import subprocess

## FIXME: Use formating strings to make the templates more readable.

header_filename = "./instructions.hxx"
data_json_filename = "./instructions.json"

data = json.load(open(data_json_filename, "r"))
header_file = open(header_filename, "w")

parameter_data_types = {
    "reg" : "RegID",
    "fl_reg" : "FL_RegID",
    "addr" : "MemPtr",
    "u8" :  "uint8_t",
    "u16" : "uint16_t",
    "u32" : "uint32_t",
    "i8" : "int8_t",
    "i16" : "int16_t",
    "i32" : "int32_t",
    "float" : "float"
}

parse_functions = {
    "reg" : "read_valid_regid",
    "fl_reg" : "read_valid_float_regid",
    "addr" : "read_valid_mem_address",
    "u8" : "read_valid_int_immediate_val<uint8_t>",
    "u16" : "read_valid_int_immediate_val<uint16_t>",
    "u32" : "read_valid_int_immediate_val<uint32_t>",
    "i8" : "read_valid_int_immediate_val<int8_t>",
    "i16" : "read_valid_int_immediate_val<int16_t>",
    "i32" : "read_valid_int_immediate_val<int32_t>",
    "float" : "read_valid_float_immediate_val"
}

hw = header_file.write

idx = 0

# Header guards and includes

hw("#ifndef INSTRUCTIONS_HXX\n")
hw("#define INSTRUCTIONS_HXX\n")
hw("#include <array>\n")
hw("#include <cstdint> \n\n")


# Generate enumerations for opcodes

hw("struct OpCodes {\n")

for i in data["instructions"].keys():
    # i = i.replace("\n", "")
    hw("\tstatic const uint8_t " + i + " = " + str(idx) + ";\n")
    idx += 1


hw("};\n\n")

# Generate instruction keyword table

hw("std::array<const char*, " + str(len(data["instructions"].keys())) + "> instruction_keywords = {\n")

for i in data["instructions"].keys():
    hw("\t\"" + data["instructions"][i]["keyword"] + "\",\n")

hw("};\n\n")

# Generate declarations of callback functions that implement instruction logic

hw("namespace callbacks {\n\n")

for i in data["instructions"]:
    keyword = data["instructions"][i]["keyword"]
    hw("\tvoid " + keyword + "_cb" + "(Interpreter& vm")
    for arg_name in data["instructions"][i]["args"].keys():
        arg_type = data["instructions"][i]["args"][arg_name]
        hw(", " + parameter_data_types[arg_type] + " " + arg_name)

    hw(");\n\n")

hw("}\n\n")

## Generate function that parses and executes the next instruction

run_next_instruction_code = """
void run_next_instruction (Interpreter &interp) {

   auto& pc = interp.m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG];
   auto& mem = interp.m_mb.memory;

   uint8_t opcode = mem[pc++];

   switch (opcode) {
    %s
    default:
          throw std::runtime_error(
              (boost::format("Invalid instruction (OPCODE: %%1\%%, PC: %%2\%%)") %%
               mem[pc] %% pc)
                  .str());
          break;

   }
}
"""

switch_cases_code = ""

for i in data["instructions"]:
    switch_cases_code += ("case OpCodes::" + i + ": {\n")
    instruction = data["instructions"][i]
    for arg_name in instruction["args"]:
        arg_type = instruction["args"][arg_name]
        switch_cases_code += (parameter_data_types[arg_type] + " " + arg_name + " = " + parse_functions[arg_type] + "(mem, pc);\n")
        switch_cases_code += ("pc += sizeof(" + parameter_data_types[arg_type] + ");")
    switch_cases_code += ("callbacks::" + instruction["keyword"] + "_cb(interp")
    for arg_name in instruction["args"]:
        switch_cases_code += (", " + arg_name)

    switch_cases_code += (");\nbreak;\n}\n")

hw(run_next_instruction_code % switch_cases_code)

hw("#endif //INSTRUCTIONS_HXX")
header_file.flush()
header_file.close()

# We might use stdin to send the code directly to the process and than let it write it to a file

clang_format_cmd = "clang-format -i " + header_filename

cres = subprocess.call(clang_format_cmd, shell = True)
if not cres: print ("Clang format successful!")
else: print("Clang format unsuccessful!")
