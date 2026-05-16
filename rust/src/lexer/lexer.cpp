#pragma once

#include "lexer.hpp"

#include <charconv>

#include <common/diagnostic.hpp>

namespace lexer {

const std::unordered_map<std::string_view, TokenType> kKeywords = {
    // branches
    {"let", TokenType::Let}, {"mut", TokenType::Mut}, {"const", TokenType::Const},
    {"volatile", TokenType::Volatile}, {"static", TokenType::Static},
    {"as", TokenType::As}, {"func", TokenType::Func}, {"inline", TokenType::Inline},
    {"lazy", TokenType::Lazy}, {"return", TokenType::Return},
    {"if", TokenType::If}, {"else", TokenType::Else}, {"while", TokenType::While},
    {"loop", TokenType::Loop}, {"for", TokenType::For}, {"in", TokenType::In},
    {"break", TokenType::Break}, {"continue", TokenType::Continue},

    // modules & OOP
    {"module", TokenType::Module}, {"export", TokenType::Export},
    {"import", TokenType::Import}, {"extern", TokenType::Extern},
    {"using", TokenType::Using}, {"self", TokenType::Self},
    {"enum", TokenType::Enum}, {"match", TokenType::Match},
    {"struct", TokenType::Struct}, {"ref", TokenType::Ref},
    {"trait", TokenType::Trait}, {"impl", TokenType::Impl},
    {"where", TokenType::Where}, {"requires", TokenType::Requires},

    // typeinfo
    { "sizeof", TokenType::Sizeof }, { "alignof", TokenType::AlignOf },
  { "decltype", TokenType::Decltype }, { "reflect", TokenType::Reflect },

    // safety and visibility
    {"public", TokenType::Public}, {"private", TokenType::Private},
    {"protected", TokenType::Protected}, {"safe", TokenType::Safe},
    {"unsafe", TokenType::Unsafe},

    // concurrency
    {"async", TokenType::Async}, {"await", TokenType::Await},
    {"dyn", TokenType::Dyn}, {"new", TokenType::New},
    {"box", TokenType::Box}, {"move", TokenType::Move},

    // primitive types
    {"i8", TokenType::Int8}, {"i16", TokenType::Int16}, {"i32", TokenType::Int32},
    {"i64", TokenType::Int64}, {"i128", TokenType::Int128},
    {"u8", TokenType::UInt8}, {"u16", TokenType::UInt16}, {"u32", TokenType::UInt32},
    {"u64", TokenType::UInt64}, {"u128", TokenType::UInt128},
    {"f32", TokenType::Float32}, {"f64", TokenType::Float64},
    {"isize", TokenType::ISize}, {"usize", TokenType::USize},
    {"bool", TokenType::Bool}, {"str", TokenType::String},
    {"unit", TokenType::Unit},

    // literals
    {"true", TokenType::LiteralBool}, {"false", TokenType::LiteralBool},

    // syntax sugar
    {"and", TokenType::LogicAnd}, {"or", TokenType::LogicOr}, {"not", TokenType::LogicNot}
};

Lexer::Lexer(std::string_view source, std::string file_name)
    : source_(source), location_({std::move(file_name)}) {}

char Lexer::Peek(std::size_t offset) const {
  if (pos_ + offset >= source_.size()) {
    return '\0';
  }

  return source_[pos_ + offset];
}

char Lexer::Consume() {
  char c = source_[pos_++];
  location_.Advance(c);

  return c;
}

void Lexer::SkipWhitespaceAndComments() {
  while (true) {
    auto c = Peek();

    if (std::isspace(c)) {
      Consume();
    } else if (c == '/' && Peek(1) == '/') { // single line comment
      while (Peek() != '\n' && Peek() != '\0') {
        Consume();
      }
    } else if (c == '/' && Peek(1) == '*') { // multi line comment
      Consume();
      Consume();

      while (not (Peek() == '*' && Peek(1) == '/') && Peek() != '\0') {
        Consume();
      }

      if (Peek() != '\0') {
        Consume(); Consume();
      }
    } else {
      break;
    }
  }
}

bool Lexer::IsEof() const {
  return pos_ >= source_.size();
}

bool Lexer::Match(char expected) {
  if (IsEof() || Peek() != expected) {
    return false;
  }

  Consume();
  return true;
}

common::Tryable<Token> Lexer::NextToken() {
  SkipWhitespaceAndComments();

  if (IsEof()) {
    return Token{TokenType::Eof, std::monostate{}, location_};
  }

  auto c = Peek();

  if (std::isalpha(c) || c == '_') {
    return ReadIdentifier();
  }

  if (std::isdigit(c)) {
    return ReadNumber();
  }

  if (c == '"') {
    return ReadString();
  }

  common::SourceLocation start_loc = location_;
  Consume();

#define RETURN_TOKEN(token_type) \
  return Token{token_type, std::monostate{}, start_loc}

  switch (c) {
    case ':':
      if (Match(':'))
        RETURN_TOKEN(TokenType::DoubleColon);
      RETURN_TOKEN(TokenType::Colon);

    case ';': RETURN_TOKEN(TokenType::Semi);
    case ',': RETURN_TOKEN(TokenType::Comma);
    case '?': RETURN_TOKEN(TokenType::Question);
    case '#': RETURN_TOKEN(TokenType::Hash);
    case '$': RETURN_TOKEN(TokenType::Dollar);
    case '@': RETURN_TOKEN(TokenType::At);
    case '~': RETURN_TOKEN(TokenType::Tilda);
    case '^':
      if (Match('='))
        RETURN_TOKEN(TokenType::CaretEq);
      RETURN_TOKEN(TokenType::Caret);

    case '.':
      if (Match('.')) {
        if (Match('='))
          RETURN_TOKEN(TokenType::DotDotEq);
        RETURN_TOKEN(TokenType::DotDot);
      }
      RETURN_TOKEN(TokenType::Dot);

    case '=':
      if (Match('='))
        RETURN_TOKEN(TokenType::EqEq);
      if (Match('>'))
        RETURN_TOKEN(TokenType::FatArrow);
      RETURN_TOKEN(TokenType::Equal);

    case '+':
      if (Match('='))
        RETURN_TOKEN(TokenType::PlusEq);
      RETURN_TOKEN(TokenType::Plus);

    case '-':
      if (Match('>'))
        RETURN_TOKEN(TokenType::Arrow);
      if (Match('='))
        RETURN_TOKEN(TokenType::MinusEq);
      RETURN_TOKEN(TokenType::Minus);

    case '*':
      if (Match('='))
        RETURN_TOKEN(TokenType::StarEq);
      RETURN_TOKEN(TokenType::Star);

    case '/':
      if (Match('='))
        RETURN_TOKEN(TokenType::SlashEq);
      RETURN_TOKEN(TokenType::Slash);

    case '!':
      if (Match('='))
        RETURN_TOKEN(TokenType::NotEq);
      RETURN_TOKEN(TokenType::Exclamation);

    case '&':
      if (Match('&'))
        RETURN_TOKEN(TokenType::AndAnd);
      if (Match('='))
        RETURN_TOKEN(TokenType::AmpersandEq);
      RETURN_TOKEN(TokenType::Ampersand);

    case '|':
      if (Match('|'))
        RETURN_TOKEN(TokenType::PipePipe);
      if (Match('='))
        RETURN_TOKEN(TokenType::PipeEq);
      RETURN_TOKEN(TokenType::Pipe);

    case '<':
      if (Match('<')) {
        if (Match('='))
          RETURN_TOKEN(TokenType::LessLessEq);
        RETURN_TOKEN(TokenType::LessLess);
      }
      if (Match('=')) {
        if (Match('>'))
          RETURN_TOKEN(TokenType::Spaceship); // <=>
        RETURN_TOKEN(TokenType::LessEq);
      }
      RETURN_TOKEN(TokenType::Less);

    case '>':
      if (Match('>')) {
        if (Match('='))
          RETURN_TOKEN(TokenType::GreaterGreaterEq);
        RETURN_TOKEN(TokenType::GreaterGreater);
      }
      if (Match('='))
        RETURN_TOKEN(TokenType::GreaterEq);
      RETURN_TOKEN(TokenType::Greater);

    case '\'': return ReadTickOrChar();

    case '(': RETURN_TOKEN(TokenType::LParen);
    case ')': RETURN_TOKEN(TokenType::RParen);
    case '{': RETURN_TOKEN(TokenType::LBrace);
    case '}': RETURN_TOKEN(TokenType::RBrace);
    case '[': RETURN_TOKEN(TokenType::LBracket);
    case ']': RETURN_TOKEN(TokenType::RBracket);

    default:
      common::DiagnosticEngine::GetInstance().Report(common::DiagnosticLevel::Error, start_loc, "Unknown character");
      RETURN_TOKEN(TokenType::Invalid);
  }
#undef RETURN_TOKEN
}

// keywords
common::Tryable<Token> Lexer::ReadIdentifier() {
  auto start_loc = location_;
  auto start_pos = pos_;

  while (not IsEof() && (std::isalnum(Peek()) || Peek() == '_')) {
    Consume();
  }

  auto text = source_.substr(start_pos, pos_ - start_pos);

  if (text == "_") {
    return Token{TokenType::Underscore, std::monostate{}, start_loc};
  }

  if (auto it = keywords.find(text); it != keywords.end()) {
    // true/false
    if (it->second == TokenType::LiteralBool) {
      return Token{it->second, (text == "true"), start_loc};
    }

    // other
    return Token{it->second, std::monostate{}, start_loc};
  }

  return Token{TokenType::Identifier, std::string(text), start_loc};
}

common::Tryable<Token> Lexer::ReadNumber() {
  auto start_loc = location_;
  auto start_pos = pos_;
  bool is_float = false;

  while (not IsEof()) {
    auto c = Peek();
    if (std::isdigit(c)) {
      Consume();
    } else if (c == '.') {
      // '1..10'
      if (Peek(1) == '.') {
        break;
      }

      if (is_float) {
        break;
      }

      is_float = true;
      Consume();
    } else {
      break;
    }
  }

  auto num_part = source_.substr(start_pos, pos_ - start_pos);
  std::string_view suffix;

  if (Match('_')) { // 42_u64
    auto s_start = pos_;
    while (not IsEof() && std::isalnum(Peek())) {
      Consume();
    }

    suffix = source_.substr(s_start, pos_ - s_start);
  }

  return FinalizeNumericToken(num_part, suffix, is_float, start_loc);
}

template <typename T>
std::optional<T> ParseString(std::string_view str) {
  T value;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
  if (ec == std::errc{}) {
    return value;
  }

  return std::nullopt;
}

template <typename T>
std::optional<T> ParseInt128(std::string_view str) {
  T result = 0;
  for (auto c : str) {
    if (c < '0' || c > '9') {
      return std::nullopt;
    }
    T next = result * 10 + (c - '0');
    if (next < result) {
      return std::nullopt; // overflow check
    }
    result = next;
  }
  return result;
}

common::Tryable<Token> Lexer::FinalizeNumericToken(std::string_view str,
                                                   std::string_view suffix,
                                                   bool is_float,
                                                   common::SourceLocation loc) {
  // floating point
  if (is_float) {
    auto val = ParseString<double>(str);
    if (not val.has_value()) {
      common::DiagnosticEngine::GetInstance().Report(
          common::DiagnosticLevel::Error, loc, "Invalid or out-of-range float literal");

      return Token{TokenType::Invalid, {}, loc};
    }

    if (suffix == "f32") {
      return Token{TokenType::LiteralFloat, static_cast<float>(*val), loc};
    }

    // f64 default
    return Token{TokenType::LiteralFloat, *val, loc};
  }

  // integer
  std::string_view effective_suffix = suffix;
  if (suffix == "isize") effective_suffix = (sizeof(void*) == 8) ? "i64" : "i32";
  if (suffix == "usize") effective_suffix = (sizeof(void*) == 8) ? "u64" : "u32";


  auto report_overflow = [&]() {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error, loc, "Integer literal is out of range for the specified type");
    return Token{TokenType::Invalid, {}, loc};
  };

  if (effective_suffix == "u8") {
    if (auto v = ParseString<std::uint8_t>(str)) {
      return Token{TokenType::LiteralInt, *v, loc};
    }

    return report_overflow();
  }
  if (effective_suffix == "u16") {
    if (auto v = ParseString<std::uint16_t>(str)) {
      return Token{TokenType::LiteralInt, *v, loc};
    }

    return report_overflow();
  }
  if (effective_suffix == "u32") {
    if (auto v = ParseString<std::uint32_t>(str)) {
      return Token{TokenType::LiteralInt, *v, loc};
    }

    return report_overflow();
  }
  if (effective_suffix == "u64") {
    if (auto v = ParseString<std::uint64_t>(str)) {
      return Token{TokenType::LiteralInt, *v, loc};
    }

    return report_overflow();
  }
  if (effective_suffix == "u128") {
    if (auto v = ParseInt128<__uint128_t>(str)) {
      return Token{TokenType::LiteralInt, *v, loc};
    }

    return report_overflow();
  }

  if (effective_suffix == "i8") {
    if (auto v = ParseString<std::int8_t>(str)) {
      return Token{TokenType::LiteralInt, *v, loc};
    }

    return report_overflow();
  }
  if (effective_suffix == "i16") {
    if (auto v = ParseString<std::int16_t>(str)) {
      return Token{TokenType::LiteralInt, *v, loc};
    }

    return report_overflow();
  }
  if (effective_suffix == "i64") {
    if (auto v = ParseString<int64_t>(str)) return Token{TokenType::LiteralInt, *v, loc};
    return report_overflow();
  }
  if (effective_suffix == "i128") {
    if (auto v = ParseInt128<__int128_t>(str)) return Token{TokenType::LiteralInt, *v, loc};
    return report_overflow();
  }

  // default - i32

  if (auto v = ParseString<std::int32_t>(str)) {
    return Token{TokenType::LiteralInt, *v, loc};
  }

  return report_overflow();
}

///TODO: support utf-8
common::Tryable<Token> Lexer::ReadString() {
  auto start_loc = location_;
  Consume(); // consuming opening "

  std::string content;
  while (!IsEof() && Peek() != '\"') {
    char c = Consume();

    if (c == '\\') { // escape sequence
      if (IsEof()) break;
      char escaped = Consume();
      switch (escaped) {
        case 'n':  content += '\n'; break;
        case 't':  content += '\t'; break;
        case 'r':  content += '\r'; break;
        case '\\': content += '\\'; break;
        case '\"': content += '\"'; break;
        case '0':  content += '\0'; break;
        default:   common::DiagnosticEngine::GetInstance()
                  .Report(
                    common::DiagnosticLevel::Error,
                    start_loc,
                    "Unknown escape sequence literal: \\" + std::string{escaped}
                  );
                  return Token{TokenType::Invalid, {}, start_loc};
      }
    } else {
      content += c;
    }
  }

  if (!Match('\"')) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error, start_loc, "Unterminated string literal");
    return Token{TokenType::Invalid, std::monostate{}, start_loc};
  }

  return Token{TokenType::LiteralString, std::move(content), start_loc};
}

common::Tryable<Token> Lexer::ReadTickOrChar() {
  auto start_loc = location_;
  Consume(); // consuming opening '

  if (IsEof()) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error, start_loc, "Empty character literal or lifetime");
    return Token{TokenType::Invalid, {}, start_loc};
  }

  if (std::isalpha(Peek()) || Peek() == '_') {
    auto start_pos = pos_;
    while (not IsEof() && (std::isalnum(Peek()) || Peek() == '_')) {
      Consume();
    }

    auto name = source_.substr(start_pos, pos_ - start_pos);

    if (Match('\'')) { // CharLiteral
      if (name.size() > 1) { // 'abc' is error
          common::DiagnosticEngine::GetInstance().Report(
              common::DiagnosticLevel::Error, start_loc, "Character literal must contain exactly one character");
      }
      return Token{TokenType::LiteralChar, static_cast<std::int32_t>(name[0]), start_loc};
    }

    return Token{TokenType::Tick, std::string(name), start_loc};
  }

  char32_t content;
  if (Peek() == '\\') {
    Consume(); // \.
    switch (auto escaped = Consume()) {
      case 'n': content = '\n'; break;
      case 't': content = '\t'; break;
      case 'r': content = '\r'; break;
      case '\'': content = '\''; break;
      case '\\': content = '\\'; break;
      case '0': content = '\0'; break;
      default: content = escaped;
    }
  } else {
    content = Consume();
  }

  if (not Match('\'')) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error, start_loc, "Unterminated character literal");
    return Token{TokenType::Invalid, {}, start_loc};
  }

  return Token{TokenType::LiteralChar, static_cast<std::int32_t>(content), start_loc};
}

}  // namespace lexer