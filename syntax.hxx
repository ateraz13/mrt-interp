#ifndef SYNTAX_HXX
#define SYNTAX_HXX
#include "parse.hxx"
#include <cstring>
#include <vector>

/*
  NOTE: Although the values are parsed during the tokenization, during the runtime
  the values will be stored in memory and most likely the interpreter will hold references
  to memory addresses where these values are stored, the memory bank is responsible for this task.

  Interpreter can be passed down into other classes in order its resources accessible to them.
*/
namespace math_ling {

  struct Value {

  };

  struct Term {
    std::vector<Value> values;
  };

  struct SetOfTerms {
    std::vector<std::pair<bool, Term>> terms;
  };
}

struct TokenizeError {

  enum Type {
    Nothing = 0,
    FailedParsing
  } type;

};

struct ExpressionToken {
  enum Type {
    NUMBER, OPERATOR, IDENTIFIER
  } type;

  union Value {
    double number;
    MathOperator math_operator;
    // NOTE: All identifiers will be stored in a single block of memory
    const char* identifier;

    Value() { std::memset(this, 0, sizeof(Value)); }
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

// I would much prefer for this function to be in the source file but MSVC seems to not like specialization of templates.
// The program doesn't seem to link under MSVC if the implementation is in the source file.
template<class Container>
TokenizeError tokenize(CharIter begin_iter, CharIter end_iter, Container& output) {

  enum TokenizeState {
    LOOKING_FOR_NUMBER, LOOKING_FOR_OPERATOR
  } ts = LOOKING_FOR_NUMBER;

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

    } catch (ParseError pe) {
      std::cout << "Error while parsing: " << pe.what() << std::endl;
      return TokenizeError { .type = TokenizeError::Type::FailedParsing };
    }
  }
  return TokenizeError { .type = TokenizeError::Type::Nothing };
}

std::ostream& operator<<(std::ostream& out_strm, ExpressionToken expr);

#endif // SYNTAX_HXX
