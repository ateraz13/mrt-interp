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

template<template<class> class Container>
extern TokenizeError tokenize(CharIter begin_iter, CharIter end_iter, Container<ExpressionToken>& output);

std::ostream& operator<<(std::ostream& out_strm, ExpressionToken expr);

#endif // SYNTAX_HXX
