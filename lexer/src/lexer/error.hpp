#pragma once

#include <string>

enum class ErrorType {
  LexicalError,
  ParseError,
  RuntimeError
};

struct Error {
  ErrorType type;
  std::string message;
};