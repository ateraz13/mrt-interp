#include <interp/assembler/assembler.hxx>
#include <cstdint>
#include <interp/instructions.hxx>
#include <iterator>
#include <string>
#include <vector>

/*
** TODO: Don't forget to handle label offsets correctly.
** To be honest we are running the code without any kernel so it most addresses
** will be hardcoded anyway
*/

assembler::Assembler &assembler::Assembler::operator<<(const std::string &str) {
  std::string line;

  return *this;
}
