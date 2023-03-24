#include "parse.hxx"
#include <algorithm>
#include <boost/format.hpp>
#include <cmath>
#include <iostream>
#include <sstream>

using fmt = boost::format;

// NOTE: This function is being called several times in a row, first in the
// tokenize function and then in the parse functions one after another.
void skip_whitespaces(CharIter &begin_iter, CharIter end_iter) {
  while (begin_iter != end_iter && *begin_iter == ' ') {
    begin_iter++;
  }
}

CharIter find_last_digit(CharIter begin_iter, CharIter end_iter) {
  while (*begin_iter >= '0' && *begin_iter <= '9' && begin_iter != end_iter) {
    begin_iter++;
  }
  return begin_iter;
}

bool is_space(char c) {
  switch (c) {
  case ' ':
  case '\t':
    return true;
  default:
    return false;
  }
}

bool is_digit(char c) {
  if (c >= '0' && c <= '9') {
    return true;
  } else {
    return false;
  }
}

std::pair<bool, CharIter> find_decimal_point_before_space(CharIter begin_iter,
                                                          CharIter end_iter) {
  while (begin_iter != end_iter && !is_space(*begin_iter)) {
    if (*begin_iter == '.') {
      return {true, begin_iter};
    } else if (!is_digit(*begin_iter)) {
      auto msg =
          fmt("Expected floating point literal\nInvalid character: %1%") %
          *begin_iter;
      throw ParseError(msg.str());
    }
    begin_iter++;
  }
  return {false, begin_iter};
}

std::pair<MathOperator, CharIter> parse_operator(CharIter begin_iter,
                                                 CharIter end_iter) {
  MathOperator op{MathOperator::NONE};

  skip_whitespaces(begin_iter, end_iter);

  if (begin_iter == end_iter) {
    return {op, end_iter};
  }

  switch (*begin_iter) {
  case '+':
    op = MathOperator::PLUS;
    break;
  case '-':
    op = MathOperator::MINUS;
    break;
  case '/':
    op = MathOperator::DIV;
    break;
  case '*':
    op = MathOperator::MULT;
    break;
  default:
    auto msg = fmt("Expected an operator found %1%") % *begin_iter;
    throw ParseError(msg.str());
    break;
  }

  return {op, ++begin_iter};
}

std::pair<int, CharIter> parse_int(CharIter begin_iter, CharIter end_iter) {
  int count = 1;
  int sum = 0;

  skip_whitespaces(begin_iter, end_iter);

  bool is_negative = *begin_iter == '-' ? true : false;
  is_negative ? begin_iter++ : begin_iter; // increment iter

  auto last_digit = find_last_digit(begin_iter, end_iter);

  for (auto &i = begin_iter; i != last_digit; i++) {
    auto c = *i;

    if (!is_digit(c)) {
      auto msg = fmt("Expected integer literal!\nInvalid digit: '%1%'") % c;
      throw ParseError(msg.str());
    }

    int n = c - '0';
    sum = sum * 10 + n;
  }

  if (begin_iter != end_iter && !is_space(*begin_iter)) {
    auto msg =
        fmt("Unexpected symbol after integer literal: '%1%'") % *begin_iter;
    throw ParseError(msg.str());
  }

  return {is_negative ? -sum : sum, begin_iter};
}

std::pair<double, CharIter> parse_float_decimal_part(CharIter begin_iter,
                                                     CharIter end_iter) {

  float decimal_place = 0.1f;
  auto &i = begin_iter;
  float sum = 0.0f;

  if (*begin_iter == '.') {
    begin_iter++;

    while (begin_iter != end_iter) {

      char c = *i;

      if (!is_digit(c)) {
        auto msg = fmt("Expected digit after decimal point while "
                       "parsing float! (FOUND:  %1%)") %
                   c;
        throw ParseError(msg.str());
      }

      sum = sum + decimal_place * c;
      decimal_place *= 0.1f;

      return {sum, begin_iter};
    }
  } else {
    return {sum, begin_iter};
  }

  return {sum, i};
}

std::pair<double, CharIter> parse_float(CharIter begin_iter,
                                        CharIter end_iter) {
  int count = 1;
  double sum = 0;

  skip_whitespaces(begin_iter, end_iter);

  bool is_negative = *begin_iter == '-' ? true : false;
  is_negative ? begin_iter++ : begin_iter; // increment iter

  try {
    auto res = parse_int(begin_iter, end_iter);
    sum = res.first;
    begin_iter = res.second;
  } catch (ParseError err) {
    auto msg = fmt("Failed while parsing integer part of float with: \n%1%") %
               err.what();
    throw ParseError(msg.str());
  }

  if (begin_iter != end_iter) {
    try {
      auto res = parse_float_decimal_part(begin_iter, end_iter);
      sum += res.first;
      begin_iter = res.second;

    } catch (ParseError err) {
      auto msg =
          fmt("Failed while parsing decimal part of a float with: \n %1%") %
          err.what();
      throw ParseError(msg.str());
    }
  }

  if (begin_iter != end_iter && !is_space(*begin_iter)) {
    auto msg =
        fmt("Unexpected symbol after integer literal: '%1%'") % *begin_iter;
    throw ParseError(msg.str());
  }

  return {is_negative ? -sum : sum, begin_iter};
}

std::ostream &operator<<(std::ostream &out_strm, MathOperator op) {
  switch (op) {
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
