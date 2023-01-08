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

class CodeGenerator:

    def flatten(self, lst, separator=""):
        if type(lst) == str:
            return lst
        elif type(lst) == list:
            if len(lst) > 0:
                string = lst.pop(0)
                for s in lst:
                    string += separator + str(s)
                return string;
            else:
                return ""
        elif lst == None:
            return ""
        elif type(lst) == None:
            return ""
        else:
            return str(lst)

    def generate_local_includes(self, includes):
        includes_src = ""
        for include in includes:
            includes_src += "#include \"" + include + "\"\n"
        return includes_src

    def generate_global_includes(self, includes):
        includes_src = ""
        for include in includes:
            includes_src += "#include <" + include + ">\n"
        return includes_src

    def generate_header_guard(self, name, contents):
        return """\
        #ifndef %s_HXX
        #define %s_HXX

        %s

        #endif // %s
        """ % (name, name, self.flatten(contents), name)

    def generate_namespace(self, name, contents):
        return """\
        namespace %s {
            %s
        } // sauce
        """ % (name, self.flatten(contents))

    def generate_class(self, name, contents):
        return """\
        class %s {
            %s
        };
        """ % (name, self.flatten(contents))

    def generate_struct(self, name, contents):
        return """\
        struct %s {
            %s
        };
        """ % (name, self.flatten(contents))

    def generate_struct_template(self, name, type_defs, contents):
        return """
        template<%s>
        struct %s {
            %s
        };
        """ % (self.flatten(type_defs, separator=", "), name, self.flatten(contents))

    def generate_struct_template_specialization(self, type_defs, name, contents):
        return """
        template<>
        struct %s<%s> {
            %s
        };
        """ % (name, self.flatten(type_defs, separator=", "), self.flatten(contents))

    def generate_array(self, data_type, array_size, array_name, contents):
        return """
        std::array<%s, %d> %s {
            %s
        };
        """ % (data_type, array_size, array_name, self.flatten(contents))

    def generate_variant(self, data_types, name):
        variant = "std::variant<%s> %s;"
        type_specifiers = self.flatten(data_types, separator=", ")
        return variant % (type_specifiers, name)


class InstructionGenerator(CodeGenerator):

    header_filename = "./instructions.hxx"
    source_filename = "./instructions.cxx"
    data_json_filename = ""

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

    parameter_variaties = []
    opcode_enums = []

    def __init__(self, json_fn):
        self.data_json_filename = json_fn
        self.data = json.load(open(self.data_json_filename, "r"))
        self.header_file = open(self.header_filename, "w")
        self.source_file = open(self.source_filename, "w")

        self.opcode_enums = list(self.data["instructions"].keys())
        self.compile_parameter_variaties()

        header_file_src = self.flatten([
            self.generate_header_guard("INSTRUCTIONS", [
                self.generate_local_includes(["interpreter.hxx"]),
                self.generate_global_includes(["array", "cstdint", "variant"]),
                self.generate_namespace("VM", [
                    self.generate_struct("OpCodes", [
                        self.generate_opcode_enumerations(),
                    ]),
                    self.generate_instruction_keyword_array(),
                    # self.generate_parameter_lists_array(),
                    self.generate_namespace("parameters", [
                        self.generate_parameter_list_types(),
                        self.generate_parameter_variant(),
                        self.generate_parameter_parser()
                    ]),
                    self.generate_namespace("callbacks", [
                        self.generate_callback_declarations(),
                    ]),
                ])
            ])
        ])

        source_file_src = self.flatten([
            self.generate_instruction_executor()
        ])

        self.header_file.write(header_file_src)
        self.header_file.flush()
        self.header_file.close()

        self.source_file.write(source_file_src)
        self.source_file.flush();
        self.source_file.close();

        self.generate_instruction_executor()

    def compile_parameter_variaties(self):
        self.parameter_variaties = [x["args"] for x in self.data["instructions"].values()]

    def generate_parameter_list_types(self):
        structs = self.generate_struct_template("ParameterList", "uint8_t", "")

        ## FIXME Because we have compiled all the parameter lists to only the unique ones now we need to find a way
        ## to map OpCode to its parameter type.
        ##
        ## TODO: Instead of compiling to unique parameter lists we can just generate all the parameter lists they will occupy
        ## the same amount of memory anyway once they are part of a variant. Unless there is a good reason not to, generating two paramter lists with
        ## the same parameter types shouldn't be a problem and as a added benefit we can name our parameters accordingly instead of using names like "arg1, arg2".

        structs += "template<uint8_t OpCode> ParameterList<OpCode> parse (MemoryBank::MemoryBuffer &buffer, uint32_t &pc);"

        for i, pv in enumerate(self.parameter_variaties):
            opcode = "OpCodes::"+self.opcode_enums[i]
            if len(pv.keys()) == 0:
                structs += self.generate_struct_template_specialization(opcode, "ParameterList", "")
            else:
                structs += self.flatten(self.generate_struct_template_specialization(opcode, "ParameterList", [
                    "\t" + self.parameter_data_types[data_type] + " " + name +";\n"  for name, data_type in zip(pv.keys(), pv.values())
                ]))

            parse_args = ["args." + name  + " = " + self.parse_functions[data_type] + "(buffer, pc);\n" for name, data_type in zip(pv.keys(), pv.values())]

            structs += """\n
                template<>
                ParameterList<%s> parse<%s>(MemoryBank::MemoryBuffer &buffer, uint32_t &pc) {
                    ParameterList<%s> args;
                    %s
                    return args;
                }
            """ % (opcode, opcode, opcode, self.flatten(parse_args))


        return structs

    def generate_parameter_variant(self):
        return "using ParameterListAny = std::variant<\n%s\n>;" \
                % self.flatten(["ParameterList<OpCodes::"+self.opcode_enums[x]+">\n "
                                for x in range(0, len(self.parameter_variaties))], separator=", ")


    def generate_argument_parser(self):
        # Generate parameter parsers for each instruction
        return ""

    def generate_opcode_enumerations(self):
        idx = 0
        opcode_enumerations = ""

        for i in self.data["instructions"].keys():
            # i = i.replace("\n", "")
            opcode_enumerations += "\tstatic const uint8_t " + i + " = " + str(idx) + ";\n"
            idx += 1

        return opcode_enumerations

    def generate_instruction_keyword_array(self):
        keyword_count = len(self.data["instructions"].keys())
        instruction_keyword_str_literals = ""

        for i in self.data["instructions"].keys():
            instruction_keyword_str_literals += "\t\"" + self.data["instructions"][i]["keyword"] + "\",\n"

        return self.generate_array("const char*", keyword_count, "instruction_keywords", instruction_keyword_str_literals)

    def generate_parameter_lists_array(self):
        return ""

    def generate_callback_declarations(self):

        cb = """
        void %s_cb(Interpreter& vm, const parameters::ParameterList<VM::OpCodes::%s>& params);
        """

        callback_declarations = self.flatten([
            cb % (self.data["instructions"][opcode]["keyword"], opcode) for opcode in self.data["instructions"]
        ])

        callback_declarations += "void run_next_instruction (Interpreter &interp);"
        return callback_declarations

    def generate_parameter_parser(self):
        source = """\n
        ParameterListAny parse_parameter(OpCode op, MemoryBank::MemoryBuffer &buffer, uint32_t &pc) {
            switch(op) {
                %s
            }
        }
        """

        case = "case OpCodes::%s: return parse<OpCodes::%s>(buffer, pc);"

        cases = self.flatten([
            case % (opcode, opcode) for opcode in self.data["instructions"].keys()
        ])

        return source % cases

    ## Generate function that parses and executes the next instruction

    def generate_instruction_executor(self):
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

        case = """\
        case VM::OpCodes::%s: {
            auto parameters = parameters::parse<VM::OpCodes::%s>(mem, pc);
            %s(parameters);
            break;
        };
        """
        switch_cases_code = self.flatten([
             case %  (opcode, opcode, self.data["instructions"][opcode]["keyword"] + "_cb") for opcode in self.data["instructions"]
        ])

        return run_next_instruction_code % switch_cases_code


# We might use stdin to send the code directly to the process and than let it write it to a file

def clang_format_file(fn):
    clang_format_cmd = "clang-format -i " + fn
    cres = subprocess.call(clang_format_cmd, shell = True)
    if not cres: print ("clang-format -i  " + fn + ": success!")
    else: print ("clang-format -i  " + fn + ": failed!")


ins_gen = InstructionGenerator("./instructions.json")
clang_format_file(ins_gen.header_filename)
clang_format_file(ins_gen.source_filename)
