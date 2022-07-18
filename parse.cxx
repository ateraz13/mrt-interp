#include "parse.hxx"
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iostream>

// NOTE: This function is being called several times in a row, first in the tokenize function
// and then in the parse functions one after another.
void skip_whitespaces(CharIter& begin_iter, CharIter end_iter) {
  while(begin_iter != end_iter && *begin_iter == ' ') {
    begin_iter++;
  }
}

CharIter find_last_digit(CharIter begin_iter, CharIter end_iter) {
  while(*begin_iter >= '0' && *begin_iter <= '9' && begin_iter != end_iter) {
    begin_iter++;
  }
  return begin_iter;
}

std::pair<MathOperator, CharIter> parse_operator(CharIter begin_iter, CharIter end_iter) {
  MathOperator op  { MathOperator::NONE };

  skip_whitespaces(begin_iter, end_iter);
  switch(*begin_iter) {
  case '+': op = MathOperator::PLUS; break;
  case '-': op = MathOperator::MINUS; break;
  case '/': op = MathOperator::DIV; break;
  case '*': op = MathOperator::MULT; break;
  default: op = MathOperator::NONE; break;
  }

  return {op, ++begin_iter};
}

std::pair<int, CharIter> parse_int(CharIter begin_iter, CharIter end_iter)
{
  std::stringstream strstrm;
  int count = 1;
  int sum = 0;

  skip_whitespaces(begin_iter, end_iter);

  bool is_negative = *begin_iter == '-' ? true  : false;
  is_negative ? begin_iter++ : begin_iter; // increment iter


  auto last_digit = find_last_digit(begin_iter, end_iter);

  for(auto& i = begin_iter; i != last_digit; i++) {
    auto c = *i;
    if(c < '0' || c > '9') {
      strstrm << "Error converting string to int!\nInvalid digit: '" << c << "'";
      throw ParseError(strstrm.str());
    }

    int n = c - '0';
    sum += n * pow(10, std::distance(i, last_digit) - 1);
  }

  return { is_negative ? -sum : sum, begin_iter };
}

std::pair<double, CharIter> parse_float(CharIter begin_iter, CharIter end_iter)  {
  std::stringstream strstrm;
  int count = 1;
  double sum = 0;

  skip_whitespaces(begin_iter, end_iter);

  auto above_decimal_end = std::find(begin_iter, end_iter, '.');
  if(above_decimal_end == end_iter) {
    return std::pair<double, CharIter>(parse_int(begin_iter, end_iter));
  }

  bool is_negative = *begin_iter == '-' ? true  : false;
  is_negative ? begin_iter++ : begin_iter; // increment iter

  auto& i = begin_iter;
  for(; i != above_decimal_end; i++) {
    auto c = *i;
    if(c < '0' || c > '9') {
      strstrm << "Error converting string to int!\nInvalid digit: '" << c << "'";
      throw ParseError(strstrm.str());
    }

    int n = c - '0';
    sum += n * pow(10, std::distance(i, above_decimal_end) - 1);
  }

  auto last_digit = find_last_digit(above_decimal_end+1, end_iter);
  auto decimal_size = std::distance(above_decimal_end+1, last_digit);

  for(i++; i < last_digit; i++) {
    auto c = *i;
    if(c < '0' || c > '9') {
      strstrm << "Error converting string to int!\nInvalid digit: '" << c << "'";
      throw std::runtime_error(strstrm.str());
    }

    int n = c - '0';
    sum += n * 1.0f/pow(10, decimal_size - (std::distance(i, last_digit) - 1));
    count += 1;
  }

  return { is_negative ? -sum : sum, i };
}
