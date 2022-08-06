#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include "parse.hxx"
#include "syntax.hxx"
#include <list>


// NOTE: Instruction can be stored as a function pointers in an associative
// array and excecuted there after by indexing into the array
// and calling the function the pointer is pointing to.

int main(int argc, char** argv)
{
  std::cout << "Hello there!" << std::endl;


  std::cout << "Press any key to exit..." << std::endl;
  getchar();

  return 0;
}
