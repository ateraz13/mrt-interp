import os
import json
import subprocess

## FIXME: Use formating strings to make the templates more readable.
## TODO: We can use code snipets and define a macro in source code to
## identify a part that should be replaced by generated code.
## Example:
##   void hello_world() {
##          __GENERATED_EXPRESSION_(SOME_NAME,"{type} {ident} = {value};")
##
##          SOME_NAME(1)
##          SOME_NAME(2)
##          SOME_NAME("indetifier")
##
##          __GENERATED__(hello_world_body);
##   }
##   Like this we can simply define a preprocessor macro that does nothing and the
##   compiler won't have anything to complain about.


class InstructionGenerator:

    header_filename = "./instructions.hxx"
    source_filename = "./instructions.cxx"
    data_json_filename = None

    data = None
    header_file = None
    source_file = None

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

    def __init__(json_fn):
        data_json_filename = json_fn
        data = json.load(open(data_json_filename, "r"))
        header_file = open(header_filename, "w")
        source_file = open(source_filename, "w")

        generate_beginning_of_instrucions_header()
        generate_opcode_enumerations()
        generate_instruction_keyword_array()
        generate_parameter_list_array()
        generate_callback_declarations()
        generate_ending_of_instruction_header()
        generate_instruction_executor()


    def generate_begining_of_instructions_header():
        header_begining = """\
        #ifndef INSTRUCTIONS_HXX
        #define INSTRUCTIONS_HXX
        #include "interpreter.hxx"
        #include <array>
        #include <cstdint>"
        """
        header_file.write(header_beginning)


    def generate_opcode_enumerations():
        idx = 0
        opcode_struct = """\
        namespace VM {
        struct OpCodes {
            %s
        };
        """
        opcode_enumerations = ""

        for i in data["instructions"].keys():
            # i = i.replace("\n", "")
            opcode_enumerations += "\tstatic const uint8_t " + i + " = " + str(idx) + ";\n"
            idx += 1

        header_file.write(opcode_struct % opcode_enumerations)

    def generate_instruction_keyword_array():
        keyword_count = len(data["instructions"].keys())
        instruction_keyword_array = """
        static std::array<const char*, %d> instruction_keywords = {)
            %s
        };
        """

        instruction_keyword_str_literals = ""

        for i in data["instructions"].keys():
            instruction_keyword_str_literals += "\t\"" + data["instructions"][i]["keyword"] + "\",\n"

        header_file.write(instruction_keyword_array % keyword_count % instruction_keyword_str_literals)

    def generate_parameter_lists_array():
        return

    def generate_callback_declarations():
        callbacks_namespace = """
        namespace callbacks {
            %s
            void run_next_instruction (Interpreter &interp);
        }
        """

        callback_declarations = ""

        for i in data["instructions"]:
            keyword = data["instructions"][i]["keyword"]
            callbacks_declarations += ("\tvoid " + keyword + "_cb" + "(Interpreter& vm")
            for arg_name in data["instructions"][i]["args"].keys():
                arg_type = data["instructions"][i]["args"][arg_name]
                callbacks_declarations += (", " + parameter_data_types[arg_type] + " " + arg_name)

            callbacks_declarations += (");\n\n")
        header_file.write(callbacks_namespace % callbacks_declarations)

    def generate_end_of_instructions_header():
        instructions_header_end = """
        #endif //INSTRUCTIONS_HXX
        """

        header_file.write(instructions_header_end)
        header_file.flush()
        header_file.close()

    ## Generate function that parses and executes the next instruction

    def generate_instruction_executor():
        run_next_instruction_code = """\
        #include "interpreter.hxx"
        #include "instructions.hxx"
        #include <stdexept>

        void VM::run_next_instruction (Interpreter &interp) {

        auto& pc = interp.m_mb.gp_regs_32[MemoryBank::PROGRAM_COUNTER_REG];
        auto& mem = interp.m_mb.memory;

        uint8_t opcode = mem[pc++];

        if(!interp.is_running()) {
            throw std::runtime_error("Cannot run next instruction, interpreter not running!");
        }

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
            switch_cases_code += ("case VM::OpCodes::" + i + ": {\n")
            instruction = data["instructions"][i]
            # TODO: We may print instructions and their parameters for debugging purposes.
            switch_cases_code += "std::cout << \"Running " +  instruction["keyword"] +  " instruction\" << std::endl;\n"
            for arg_name in instruction["args"]:
                arg_type = instruction["args"][arg_name]
                switch_cases_code += (parameter_data_types[arg_type] + " " + arg_name + " = " + parse_functions[arg_type] + "(mem, pc);\n")
                switch_cases_code += ("pc += sizeof(" + parameter_data_types[arg_type] + ");\n")
            switch_cases_code += ("VM::callbacks::" + instruction["keyword"] + "_cb(interp")
            for arg_name in instruction["args"]:
                switch_cases_code += (", " + arg_name)

            switch_cases_code += (");\nbreak;\n}\n")

        source_file.write(run_next_instruction_code % switch_cases_code)

        source_file.flush();
        source_file.close();

# We might use stdin to send the code directly to the process and than let it write it to a file

def clang_format_file(fn):
    clang_format_cmd = "clang-format -i " + fn
    cres = subprocess.call(clang_format_cmd, shell = True)
    if not cres: print ("clang-format -i  " + fn + ": success!")
    else: print ("clang-format -i  " + fn + ": failed!")


InstructionGenerator("./instructions.json")
clang_format_file(header_filename)
clang_format_file(source_filename)
