#ifndef ASSEMBLER_HXX
#define ASSEMBLER_HXX
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace assembler {

class Label {
  std::string name;
  uint32_t offset;
};

struct Token {
  enum {
    LABEL_SPECIFIER,
    KEYWORD,
    REGISTER_ID,
    IMMEDIATE_VALUE,
    MEMORY_ADDRESS
  } type;

  std::string value;
};

using TokenVector = std::vector<Token>;
using MaybeTokenVector = std::optional<TokenVector>;

class Tokenizer {
  enum struct State {
    STATEMENT_BEGINING,
    FOUND_LABEL_COLUMN,
    FOUND_KEYWORD,
    EXPECTING_REGISTER_ID,
    EXPECTING_IMMEDIATE_VALUE,
    FOUND_DECIMAL,
    READING_DECIMAL,
    FOUND_ARGUMENT_SEPARATOR
  };

  void clear_errors();
  void clear_all();

private:
  std::vector<std::string> errors;
};

class Assembler {
public:
  Assembler &operator<<(const std::string &str);

  MaybeTokenVector tokenize(const std::string &str);

  const std::vector<std::string> get_errors() const { return m_errors; };

private:
  std::vector<Label> m_labels;
  std::vector<uint8_t> m_bytecode;
  std::vector<std::string> m_errors;
  Tokenizer m_tokinizer;
};
} // namespace assembler

#endif // ASSEMBLER_HXX
