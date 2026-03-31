#include <fstream>
#include <iostream>
#include <sstream>

#include "user.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return 1;
  }

  std::ifstream file(argv[1]);
  if (not file) {
    std::cerr << "Cannot open file " << argv[1] << '\n';
    return 1;
  }

  std::ostringstream ss;
  ss << file.rdbuf();

  // Запускаем компиляцию и выполнение
  auto result = RunCompiler(ss.str());

  if (not result) {
    std::cerr << "Error: " << result.error().message << '\n';
    return 1;
  }

  return 0;
}