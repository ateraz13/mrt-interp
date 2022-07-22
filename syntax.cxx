#include "syntax.hxx"
#include "parse.hxx"

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

template<template<class> class Container>
TokenizeError tokenize(CharIter begin_iter, CharIter end_iter, Container<ExpressionToken>& output) {

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

// Explicitly defined
// For other containers just explicitly define them.
template TokenizeError tokenize(CharIter begin_iter, CharIter end_iter, std::vector<ExpressionToken>& output);
