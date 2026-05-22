#pragma once

#include <vector>
#include <memory>
#include <expected>
#include "token.hpp"
#include "ast.hpp"
#include "error.hpp"

class Parser {
 public:
  explicit Parser(std::vector<Token> tokens);
  std::expected<std::unique_ptr<MainFunction>, Error> Parse();

 private:
  const Token& Peek() const;
  Token Consume();
  bool Match(TokenType type);

  std::expected<std::unique_ptr<Statement>, Error> ParseStatement();
  std::expected<std::unique_ptr<Expression>, Error> ParseExpression();

 private:
  std::vector<Token> tokens_;
  size_t cursor_ = 0;
};