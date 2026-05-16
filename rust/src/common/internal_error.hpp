#pragma once

#include <string>
#include <expected>

namespace common {

enum class InternalErrorKind {
  // environmental errors
  FileSystem,        // file reading error
  ResourceExhausted, // lack of memory

  // logic error
  InvariantViolated,
  NotImplemented,    // plug function call
  NullPointerAccess,

  // phase errors
  ParserStateInconsistent, // parser recursion error
  TypeSystemError,

  // backend errors
  LLVMError,         // LLVM IR generation error
  TargetTripleError  // unsupported CPU instruction
};

struct InternalError {
  InternalErrorKind kind;
  std::string message;
  std::string compiler_file;
  std::size_t compiler_line;
};

template <typename T>
using Tryable = std::expected<T, InternalError>;

#define MAKE_INTERNAL_ERROR(kind, msg) \
  std::unexpected(InternalError{kind, msg, __FILE__, __LINE__})

}  // namespace compiler::common
