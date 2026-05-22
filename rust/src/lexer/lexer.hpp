#pragma once

#include "token.hpp"

#include <common/source_location.hpp>
#include <common/internal_error.hpp>

#include <string_view>

#include <unordered_map>

namespace lexer {

class Lexer {
 private:
  static const std::unordered_map<std::string_view, TokenType> keywords;

 public:
  Lexer(std::string_view source, std::string file_name);

  common::Tryable<Token> NextToken();

 private:
  [[nodiscard]]
  char Peek(std::size_t offset = 0) const;
  char Consume();

  void SkipWhitespaceAndComments();

  [[nodiscard]]
  bool IsEof() const;
  [[nodiscard]]
  bool Match(char c);

  common::Tryable<Token> ReadIdentifier();

  common::Tryable<Token> ReadNumber();
  common::Tryable<Token> FinalizeNumericToken(std::string_view str,
                                            std::string_view suffix,
                                            bool is_float,
                                            common::SourceLocation loc);

  common::Tryable<Token> ReadString();

  common::Tryable<Token> ReadTickOrChar(common::SourceLocation loc);

 private:
  std::string source_;
  common::SourceLocation location_;
  std::size_t pos_ = 0;
};

}  // namespace lexer