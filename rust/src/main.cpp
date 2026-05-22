#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "sema/name_resolver.hpp"
#include "sema/type_checker.hpp"
#include "sema/borrow_checker.hpp"
#include "sema/interpreter.hpp"
#include "common/diagnostic.hpp"

std::string ReadFile(const std::string& path) {
  std::ifstream file(path);

  if (not file.is_open()) {
    std::cerr << "fatal error: Could not open file '" << path << "'" << std::endl;
    std::exit(1);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "usage: compiler <source_file.rpp>" << '\n';
    return 0;
  }

  std::string file_path = argv[1];
  std::string source = ReadFile(file_path);

  auto& diagnostic = common::DiagnosticEngine::GetInstance();

  lexer::Lexer lexer(source, file_path);

  parser::Parser parser(lexer);
  auto program_ast = parser.ParseProgram();

  if (diagnostic.HasErrors()) {
    std::cerr << "Stopping due to syntax errors." << '\n';
    diagnostic.Flush();
    return 1;
  }

  sema::NameResolver resolver;
  resolver.Resolve(program_ast);

  if (diagnostic.HasErrors()) {
    diagnostic.Flush();
    return 1;
  }

  sema::TypeChecker type_checker;
  type_checker.Check(program_ast);

  if (diagnostic.HasErrors()) {
    diagnostic.Flush();
    return 1;
  }

  sema::BorrowChecker borrow_checker;
  borrow_checker.Check(program_ast);

  if (diagnostic.HasErrors()) {
    diagnostic.Flush();
    return 1;
  }

  try {
    sema::Interpreter interpreter;
    interpreter.Run(program_ast);

    auto* global_scope = resolver.GetGlobalScope();
    auto* main_sym = global_scope->Lookup("main");
    if (main_sym && main_sym->GetKind() == sema::SymbolKind::Function) {
      auto* main_func = dynamic_cast<parser::ast::nodes::FuncDecl*>(main_sym->GetDeclNode());
      if (main_func->body) {
        main_func->body->Accept(&interpreter);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Runtime panic: " << e.what() << '\n';
    return 1;
  }

  return 0;
}