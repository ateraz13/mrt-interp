import os

instructions = open("./instructions.txt", "r")
enumeration = open("./instructions.txt.enum", "w")

idx = 0
for i in instructions:
    i = i.replace("\n", "")
    enumeration.write("static const uint8_t " + i + "= " + str(idx) + ";\n")
    idx += 1
