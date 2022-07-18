#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include "parse.hxx"

std::ostream& operator<<(std::ostream& out_strm, MathOperator op) {
  switch(op) {
  case MathOperator::NONE:
    out_strm << "NONE";
    break;
  case MathOperator::PLUS:
    out_strm << "PLUS";
    break;
  case MathOperator::MINUS:
    out_strm << "MINUS";
    break;
  case MathOperator::DIV:
    out_strm << "DIV";
    break;
  case MathOperator::MULT:
    out_strm << "MULT";
    break;
  }

  return out_strm;
}


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


  friend std::ostream& operator<<(std::ostream&, ExpressionToken);

};

std::ostream& operator<<(std::ostream& out_strm, ExpressionToken expr){

  switch(expr.type) {
  case ExpressionToken::Type::NUMBER:
    out_strm << "{ TYPE: NUMBER, VALUE: " << expr.value.number << " }";
    break;
  case ExpressionToken::Type::OPERATOR:
    out_strm << "{ TYPE: OPERATOR, VALUE: " << expr.value.math_operator << " }";
    break;
    default: out_strm << "UNSUPPORTED EXPRESSION_TOKEN FOR PRINTING";
  }
  return out_strm;
}


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
  } ts = LOOKING_FOR_NUMBER;

  int count = 0;
  while(begin_iter != end_iter) {


    try {
      switch(ts) {
      case LOOKING_FOR_NUMBER: {
        auto res = parse_float(begin_iter, end_iter);
        begin_iter = res.second;
        auto token = ExpressionToken::MakeNumber(res.first);
        output.push_back(token);
        ts = LOOKING_FOR_OPERATOR;
        continue;
      }
      case LOOKING_FOR_OPERATOR: {
        auto res = parse_operator(begin_iter, end_iter);
        begin_iter = res.second;
        auto token = ExpressionToken::MakeMathOperator(res.first);
        output.push_back(token);
        ts = LOOKING_FOR_NUMBER;
        continue;
      }
      default: throw std::runtime_error("Invalid tokenize state!");
      }

      begin_iter++;
    } catch (ParseError pe) {
      std::cout << "Error while parsing: " << pe.what() << std::endl;
      return TokenizeError { .type = TokenizeError::Type::FailedParsing };
    }
  }
  return TokenizeError { .type = TokenizeError::Type::Nothing };
}

int main(int argc, char** argv)
{
  std::cout << "Hello there!" << std::endl;

  int age = 0;
  float bdi = 0.0f;
  std::string name;
  std::string input_str;

  // std::cout << "Name: ";
  // std::getline(std::cin, input_str);

  // name = input_str;

  // std::cout << "Age: ";
  // std::getline(std::cin, input_str);

  // age = parse_int(begin(input_str), end(input_str)).first;

  // std::cout << "BDI(float): ";

  // std::getline(std::cin, input_str);

  // bdi = parse_float(begin(input_str), end(input_str)).first;

  // std::cout << "Hello " << name << ", you are " << age << " years old and your BDI is " << bdi << std::endl;

  std::string expr = "10 + 10";
  std::vector<ExpressionToken> tokens;
  tokenize(begin(expr), end(expr), tokens);

  for(auto t: tokens) {
    std::cout << "Token: " << t << std::endl;
  }

  std::cout << "Press any key to exit..." << std::endl;
  getchar();

  return 0;
}
