#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <list>
#include "parse.hxx"
#include "syntax.hxx"
#include "interpreter.hxx"

// NOTE: Instruction can be stored as a function pointers in an associative
// array and excecuted there after by indexing into the array
// and calling the function the pointer is pointing to.

int main(int argc, char** argv)
{

  VirtualMachine vm;

  std::cout << "Hello there!" << std::endl;

  printf("Hello world!");

  std::cout << "Press any key to exit..." << std::endl;
  getchar();

  return 0;
}
