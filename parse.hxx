#ifndef PARSE_HXX
#define PARSE_HXX
#include <string>
#include <iostream>
#include <stdexcept>

enum struct MathOperator {
  NONE = 0, PLUS, MINUS, DIV, MULT
};


class ParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

using CharIter = decltype(std::declval<std::string>().begin());

void skip_whitespaces(CharIter& begin_iter, CharIter end_iter);
std::pair<MathOperator, CharIter> parse_operator(CharIter begin_iter, CharIter end_iter);
std::pair<int, CharIter> parse_int(CharIter begin_iter, CharIter end_iter);
std::pair<double, CharIter> parse_float(CharIter begin_iter, CharIter end_iter);

std::ostream& operator<<(std::ostream& out_strm, MathOperator op);
#endif // PARSE_HXX
