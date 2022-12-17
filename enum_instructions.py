import os
import json
import subprocess


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

hw = header_file.write

idx = 0
hw("#ifndef INSTRUCTIONS_HXX\n")
hw("#define INSTRUCTIONS_HXX\n")
hw("#include <array>\n")
hw("#include <cstdint> \n\n")
hw("struct OpCodes {\n")

for i in data["instructions"].keys():
    # i = i.replace("\n", "")
    hw("\tstatic const uint8_t " + i + " = " + str(idx) + ";\n")
    idx += 1

hw("};\n\n")
hw("std::array<const char*, " + str(len(data["instructions"].keys())) + "> instruction_keywords = {\n")

for i in data["instructions"].keys():
    hw("\t\"" + data["instructions"][i]["keyword"] + "\",\n")



hw("};\n\n")

hw("namespace callbacks {\n\n")

for i in data["instructions"]:
    keyword = data["instructions"][i]["keyword"]
    hw("\tvoid " + keyword + "_cb" + "( VirtualMachine* vm")
    for arg_name in data["instructions"][i]["args"].keys():
        arg_type = data["instructions"][i]["args"][arg_name]
        hw(", " + parameter_data_types[arg_type] + " " + arg_name)

    hw(");\n\n")

hw("}\n\n")

hw("#endif //INSTRUCTIONS_HXX")
header_file.flush()
header_file.close()

# We might use stdin to send the code directly to the process and than let it write it to a file

clang_format_cmd = "clang-format -i " + header_filename

cres = subprocess.call(clang_format_cmd, shell = True)
if not cres: print ("Clang format successful!")
else: print("Clang format unsuccessful!")
