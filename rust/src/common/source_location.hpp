#pragma once

#include <string>

namespace common {

struct SourceLocation {
  SourceLocation() = default;

  explicit SourceLocation(std::string file_name)
      : file_name(std::move(file_name)) {}

  void Advance(char c) {
    if (c == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }
  }

  std::string file_name = "unknown";
  std::size_t line = 1;
  std::size_t column = 1;
};

}  // namespace common