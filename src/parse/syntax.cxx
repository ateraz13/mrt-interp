#include <interp/parse/parse.hxx>
#include <interp/parse/syntax.hxx>

std::ostream &operator<<(std::ostream &out_strm, ExpressionToken expr) {

  switch (expr.type) {
  case ExpressionToken::Type::NUMBER:
    out_strm << "{ TYPE: NUMBER, VALUE: " << expr.value.number << " }";
    break;
  case ExpressionToken::Type::OPERATOR:
    out_strm << "{ TYPE: OPERATOR, VALUE: " << expr.value.math_operator << " }";
    break;
  default:
    out_strm << "UNSUPPORTED EXPRESSION_TOKEN FOR PRINTING";
  }
  return out_strm;
}

template <template <class> class Container>
TokenizeError tokenize(CharIter begin_iter, CharIter end_iter,
                       Container<ExpressionToken> &output) {

  enum TokenizeState {
    EXPECTING_NUMBER,
    EXPECTING_OPERATOR
  } ts = EXPECTING_NUMBER;

  using ET = TokenizeError::Type;

  auto error = [](ET type) {
    return TokenizeError{.type = type};
  };

  auto no_error = []() {
    return TokenizeError{.type = ET::Nothing};
  };

  while (begin_iter != end_iter) {

    try {
      switch (ts) {
      case EXPECTING_NUMBER: {
        auto res = parse_float(begin_iter, end_iter);
        begin_iter = res.second;
        auto token = ExpressionToken::MakeNumber(res.first);
        output.push_back(token);
        ts = EXPECTING_OPERATOR;
        continue;
      }
      case EXPECTING_OPERATOR: {
        auto res = parse_operator(begin_iter, end_iter);
        begin_iter = res.second;
        auto token = ExpressionToken::MakeMathOperator(res.first);
        output.push_back(token);
        ts = EXPECTING_NUMBER;
        continue;
      }
      default:
        return error(ET::InternalTokenizerBugOrError);
      }

    } catch (ParseError pe) {
      std::cout << "Error while parsing: " << pe.what() << std::endl;
      return error(ET::FailedParsing);
    }
  }
  return no_error();
}

// Explicitly defined
// For other containers just explicitly define them.
template TokenizeError tokenize(CharIter begin_iter, CharIter end_iter,
                                std::vector<ExpressionToken> &output);
