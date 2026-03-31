#pragma once

#include <string>
#include <vector>
#include <expected>

#include "token.hpp"
#include "error.hpp"

class Lexer {
 public:
  explicit Lexer(std::string_view source);
  std::expected<std::vector<Token>, Error> Tokenize();

 private:
  std::string_view source_;
  size_t cursor_ = 0;

  char Peek() const;
  char Consume();
  void SkipWhitespace();
  Token ReadIdentifier();
  Token ReadNumber();
};