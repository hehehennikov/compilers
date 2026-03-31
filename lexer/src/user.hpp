#pragma once

#include <expected>

#include <lexer/ast.hpp>
#include <lexer/interpreter.hpp>
#include <lexer/error.hpp>
#include <lexer/lexer.hpp>
#include <lexer/parser.hpp>
#include <lexer/print_visitor.hpp>

template <typename T>
using Expect = std::expected<T, Error>;

inline Expect<void> RunCompiler(std::string_view code) {
  return Lexer(code).Tokenize()
      .and_then([](std::vector<Token> tokens) {
        return Parser(std::move(tokens)).Parse();
      })
      .and_then([](std::unique_ptr<MainFunction> program) -> Expect<std::unique_ptr<MainFunction>> {
        if (not program) {
          return std::unexpected(Error{ErrorType::SyntaxError, "Parsing failed: empty AST"});
        }
        SymbolTreeVisitor semantic_analyzer;
        program->Accept(&semantic_analyzer);

        if (semantic_analyzer.HasErrors()) {
          std::string all_errors;
          for (const auto& err : semantic_analyzer.GetErrors()) {
            all_errors += err + "\n";
          }

          return std::unexpected(Error{ErrorType::RuntimeError, all_errors});
        }

        return std::move(program);
      })
      .transform([](const std::unique_ptr<MainFunction>& program) {
        Interpreter interpreter;
        program->Accept(&interpreter);
      });
}