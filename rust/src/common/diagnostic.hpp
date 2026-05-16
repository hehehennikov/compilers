#pragma once

#include "source_location.hpp"

#include <iostream>

namespace common {

enum class DiagnosticLevel : uint {
  Note,
  Warning,
  Error
};

struct Diagnostic {
  std::string message;
  SourceLocation loc;
  DiagnosticLevel type;
};

class DiagnosticEngine {
 private:
  DiagnosticEngine() = default;

 public:
  static DiagnosticEngine& GetInstance() {
    static DiagnosticEngine inst;
    return inst;
  }

 public:
  void Report(DiagnosticLevel level, SourceLocation loc, std::string msg) {
    diagnostics.push_back({level, loc, std::move(msg)});
    if (level == DiagnosticLevel::Error) error_occurred = true;
  }

  [[nodiscard]]
  bool HasErrors() const {
    return error_occurred;
  }

  void Flush() {
    for (const auto& d : diagnostics) {
      std::cerr << Format(d) << '\n';
    }

    diagnostics.clear();
  }

 private:
  static std::string Format(const Diagnostic& d) {
    std::string prefix = (d.type == DiagnosticLevel::Error) ? "\033[1;31merror\033[0m" : "\033[1;33mwarning\033[0m";
    return prefix + "[" + d.loc.file_name + ":" + std::to_string(d.loc.line) + ":" +
           std::to_string(d.loc.column) + "]: " + d.message;
  }

 private:
  std::vector<Diagnostic> diagnostics;
  bool error_occurred = false;
};

}  // namespace common