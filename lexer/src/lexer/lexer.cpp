#include "lexer.hpp"

#include <cctype>
#include <map>

Lexer::Lexer(std::string_view source)
    : source_(source) {}

char Lexer::Peek() const {
  if (cursor_ >= source_.size()) {
    return '\0';
  }

  return source_[cursor_];
}

char Lexer::Consume() {
  return source_[cursor_++];
}

void Lexer::SkipWhitespace() {
  while (std::isspace(Peek())) {
    Consume();
  }
}

Token Lexer::ReadIdentifier() {
  static const std::map<std::string_view, TokenType> keywords = {
    {"var", TokenType::Var},
    {"int", TokenType::IntType},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"print", TokenType::Print}};

  std::string value;
  while (std::isalnum(Peek()) or Peek() == '_') {
    value += Consume();
  }

  if (keywords.contains(value)) {
    return {keywords.at(value), value};
  }

  return {TokenType::Identifier, value};
}

Token Lexer::ReadNumber() {
  std::string value;
  while (std::isdigit(Peek())) {
    value += Consume();
  }

  return {TokenType::Number, value};
}

std::expected<std::vector<Token>, Error> Lexer::Tokenize() {
  std::vector<Token> tokens;
  while (cursor_ < source_.size()) {
    SkipWhitespace();
    if (cursor_ >= source_.size()) {
      break;
    }

    auto c = Peek();
    if (std::isalpha(c)) {
      tokens.push_back(ReadIdentifier());
    } else if (std::isdigit(c)) {
      tokens.push_back(ReadNumber());
    } else if (c == ':') {
      Consume();
      tokens.push_back({TokenType::Colon, ":"});
    } else if (c == ';') {
      Consume();
      tokens.push_back({TokenType::Semicolon, ";"});
    } else if (c == '=') {
      Consume();
      if (Peek() == '=') {
        Consume();
        tokens.push_back({TokenType::Equals, "=="});
      } else {
        tokens.push_back({TokenType::Assign, "="});
      }
    } else if (c == '(') {
      Consume();
      tokens.push_back({TokenType::LParen, "("});
    } else if (c == ')') {
      Consume();
      tokens.push_back({TokenType::RParen, ")"});
    } else if (c == '{') {
      Consume();
      tokens.push_back({TokenType::LBrace, "{"});
    } else if (c == '}') {
      Consume();
      tokens.push_back({TokenType::RBrace, "}"});
    } else {
      return std::unexpected(Error{ErrorType::LexicalError, "Unknown char"});
    }
  }

  tokens.push_back({TokenType::Eof, ""});
  return tokens;
}