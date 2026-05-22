#include <fstream>

#include <iostream>
#include <sstream>

#include <lexer/lexer.hpp>

#include <lexer/parser.hpp>

template <typename T>
using Expect = std::expected<T, Error>;

Expect<void> RunCompiler(std::string_view code) {
  return Lexer(code).Tokenize()
      .and_then([](std::vector<Token> tokens) {
          return Parser(std::move(tokens)).Parse();
      })
      .transform([](const std::unique_ptr<MainFunction>& program) {
          program->Run();
      });
}

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

  if (auto result = RunCompiler(ss.str()); not result) {
    std::cerr << "Error: " << result.error().message << '\n';

    return 1;
  }

  return 0;
}