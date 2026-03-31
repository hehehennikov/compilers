#pragma once

#include <string>

enum class ErrorType {
  LexicalError,
  ParseError,
  RuntimeError,
  SyntaxError
};

struct Error {
  ErrorType type;
  std::string message;
};