#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include "parse.hxx"
#include "syntax.hxx"

// TODO: Reorder expression tokens into mathematically accurate order.


struct RegisterBank {

};

struct MemoryBank {

};

class Interpreter {

};

struct Instruction
{
  // NOTE: Assign instruction opcodes with specific values.
  enum struct OpCode {
    PLUS_INT, MINUS_INT, DIV_INT, MULT_INT,
    PLUS_FLOAT, MINUS_FLOAT, DIV_FLOAT, MULT_FLOAT,
    PUSH_STACK, POP_STACK, LOAD_REGISTER, STORE_REGISTER,
  } op_code;

};

int main(int argc, char** argv)
{
  std::cout << "Hello there!" << std::endl;

  int age = 0;
  float bdi = 0.0f;
  std::string name;
  std::string input_str;

  std::string expr;

  std::getline(std::cin, expr);

  std::vector<ExpressionToken> tokens;
  tokenize(begin(expr), end(expr), tokens);

  for(auto t: tokens) {
    std::cout << "Token: " << t << std::endl;
  }

  std::cout << "Press any key to exit..." << std::endl;
  getchar();

  return 0;
}
