#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include "parse.hxx"

struct ExpressionToken {
  enum Type {
    NUMBER, OPERATOR
  } type;

  union Value {
    double number;
    MathOperator math_operator;

    Value() { memset(this, 0, sizeof(Value)); }
  } value;

  static ExpressionToken MakeNumber( double val ) {
    auto t = ExpressionToken { NUMBER, {} };
    t.value.number = val;
    return t;
  }

  static ExpressionToken MakeMathOperator(MathOperator op) {
    auto t = ExpressionToken { OPERATOR, {} };
    t.value.math_operator = op;
    return t;
  }
};

struct TokenList
{
public:

  template<typename Cont>
  void compile(Cont& instr_list) {
    // NOTE: Go through all the tokens and generate instructions in the correct order
    // to evaluate the expression and push them back the the instruction list.
  }
private:
  std::vector<ExpressionToken> token;
};

// NOTE: The interpreter is going to be a stack machine,
// argmuments for instructions will be suplied through the stack;

struct Instruction
{
  enum struct OpCode {
    PLUS, MINUS, DIV, MULT
  } op_code;

};

struct TokenizeError {

  enum Type {
    Nothing = 0,
    FailedParsing
  } type;

};

// NOTE: Token will not copy parts of a string but rather use referencing
// through iterators into the original string but that is only neccessary for a more generic parser.
// In a sophisticated parser there are many different types of tokens that could be extracted from a text
// and it would be impractical to parse everything in the first pass(tokenization).
// Rather we separate keywords and other forms of tokens using simple iterators and than comeback to them
// in the parsing stage to produce values out of them, we could also copy all the substrings but that would #
// require unnecessary memory allocation which we do not want due to peformance reasons.

template<typename Cont>
TokenizeError tokenize(CharIter begin_iter, CharIter end_iter, Cont& output) {
  // NOTE: Go through the string and tokenize the string'

  enum TokenizeState {
    LOOKING_FOR_NUMBER, LOOKING_FOR_OPERATOR
  } ts;

  while(begin_iter != end_iter) {

    skip_whitespaces(begin_iter, end_iter);
    // NOTE: Look for character that would indicate what is ahead

    try {
      switch(ts) {
      case LOOKING_FOR_NUMBER: {
        auto res = parse_float(begin_iter, end_iter);
        begin_iter = res.second;
        output.push_back(ExpressionToken::MakeNumber(res.first));
        ts = LOOKING_FOR_OPERATOR;
        break;
      }
      case LOOKING_FOR_OPERATOR: {
        auto res = parse_operator(begin_iter, end_iter);
        begin_iter = res.second;
        output.push_back(ExpressionToken::MakeMathOperator(res.first));
        ts = LOOKING_FOR_NUMBER;
        break;
      }
      default: throw std::runtime_error("Invalid tokenize state!");
      }
    } catch (ParseError pe) {
      std::cout << "Error while parsing: " << pe.what() << std::endl;
      return TokenizeError { .type = TokenizeError::Type::FailedParsing };
    }
    begin_iter++;
  }
}
int main(int argc, char** argv)
{
  std::cout << "Hello there!" << std::endl;

  int age = 0;
  float bdi = 0.0f;
  std::string name;
  std::string input_str;

  std::cout << "Name: ";
  std::getline(std::cin, input_str);

  name = input_str;

  std::cout << "Age: ";
  std::getline(std::cin, input_str);

  age = parse_int(begin(input_str), end(input_str)).first;

  std::cout << "BDI(float): ";

  std::getline(std::cin, input_str);

  bdi = parse_float(begin(input_str), end(input_str)).first;

  std::cout << "Hello " << name << ", you are " << age << " years old and your BDI is " << bdi << std::endl;

  std::cout << "Press any key to exit..." << std::endl;
  getchar();

  return 0;
}
