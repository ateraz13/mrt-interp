#include <interp/assembler/assembler.hxx>
#include <iostream>

namespace assembler {

int parse_digit(const char *ptr) {
  if (*ptr >= '0' && *ptr <= '9') {
    return static_cast<int>(*ptr - '0');
  } else {
    return -1;
  }
}

const char *find_last_digit(const char *ptr) {
  while (*ptr >= '0' && *ptr <= '9') {
    ptr++;
  }
  return ptr;
}

uint32_t parse_u32(const char *ptr) {
  const char *last = find_last_digit(ptr);
  uint32_t n = 0;

  while (uint32_t d = parse_digit(ptr)) {
    n *= 10;
    n += d;
    ptr++;
  }

  return n;
}

} // namespace assembler

int main(int argc, char **argv) {

  std::cout << "Hello world!" << std::endl;

  assembler::Assembler ass;

  ass << "ldi $10, r1"
      << "addi r1, $0, r2";

  return 0;
}
