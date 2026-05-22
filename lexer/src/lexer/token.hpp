#pragma once

#include <string>
#include <string_view>

enum class TokenType {
  Var,
  Identifier,
  Colon,
  IntType,
  Semicolon,
  Assign,
  Number,
  If,
  Else,
  LParen,
  RParen,
  LBrace,
  RBrace,
  Equals,
  Print,
  Eof
};

struct Token {
  TokenType type;
  std::string value;
};