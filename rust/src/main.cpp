#include <iostream>
#include <string>
#include <vector>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include <parser/ast/visitor/print.hpp>
#include "common/diagnostic.hpp"

int main() {
  std::string source = R"(
    #[inline]
    func main(argc: i32) -> i32 {
      let mut x: i32 = 42;
      let lazy y = x * 2;

      if x == 42 {
        print("Hello World");
      }

      let res = match x {
        0 => "zero",
        _ => "non-zero",
      };

      return 0;
    }

    struct Point {
      x: f64,
      y: f64
    }

    impl Point {
      func new(x: f64, y: f64) -> Point {
        return Point { x, y };
      }
    }
  )";

  std::cout << "--- Starting Compilation ---\n";

  lexer::Lexer lexer(source, "test.rs");
  parser::Parser parser(lexer);

  auto program = parser.ParseProgram();

  if (common::DiagnosticEngine::GetInstance().HasErrors()) {
    std::cerr << "Compilation failed with errors:\n";
    common::DiagnosticEngine::GetInstance().Flush();
    return 1;
  }

  std::cout << "--- Abstract Syntax Tree ---\n";
  parser::ast::visitor::PrintVisitor printer(std::cout);

  for (auto& node : program) {
    node->Accept(&printer);
    std::cout << "---------------------------\n";
  }

  std::cout << "--- Compilation Finished ---\n";
  return 0;
}