#ifndef ASSEMBLER_HXX
#define ASSEMBLER_HXX
#include <iostream>
#include <vector>

namespace assembler {

class Label {
  std::string name;
  uint32_t offset;
};

class Assembler {
public:
  Assembler &operator<<(const std::string &str);

private:
  std::vector<Label> m_labels;
  std::vector<uint8_t> m_bytecode;
};
} // namespace assembler

#endif // ASSEMBLER_HXX
