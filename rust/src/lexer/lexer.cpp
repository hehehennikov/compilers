#include "lexer.hpp"

#include <cctype>
#include <charconv>

#include <common/diagnostic.hpp>

namespace lexer {

const std::unordered_map<std::string_view, TokenType> Lexer::keywords = {
    // branches
    {"let", TokenType::Let}, {"mut", TokenType::Mut}, {"const", TokenType::Const},
    {"volatile", TokenType::Volatile}, {"static", TokenType::Static},
    {"as", TokenType::As}, {"func", TokenType::Func}, {"inline", TokenType::Inline},
    {"return", TokenType::Return},
    {"if", TokenType::If}, {"else", TokenType::Else}, {"while", TokenType::While},
    {"loop", TokenType::Loop}, {"for", TokenType::For}, {"in", TokenType::In},
    {"break", TokenType::Break}, {"continue", TokenType::Continue}, {"defer", TokenType::Defer},

    // modules & OOP
    {"module", TokenType::Module}, {"export", TokenType::Export},
    {"import", TokenType::Import}, {"extern", TokenType::Extern},
    {"using", TokenType::Using}, {"self", TokenType::Self},
    {"enum", TokenType::Enum}, {"match", TokenType::Match},
    {"struct", TokenType::Struct}, {"ref", TokenType::Ref},
    {"trait", TokenType::Trait}, {"impl", TokenType::Impl},
    {"where", TokenType::Where}, {"requires", TokenType::Requires},

    // typeinfo
    {"sizeof", TokenType::Sizeof}, {"alignof", TokenType::AlignOf},
    {"decltype", TokenType::Decltype}, {"reflect", TokenType::Reflect},

    // safety and visibility
    {"public", TokenType::Public}, {"private", TokenType::Private},
    {"protected", TokenType::Protected}, {"safe", TokenType::Safe},
    {"unsafe", TokenType::Unsafe},

    // heap
    {"dyn", TokenType::Dyn}, {"new", TokenType::New},
    {"box", TokenType::Box}, {"move", TokenType::Move},

    // primitive types
    {"i8", TokenType::Int8}, {"i16", TokenType::Int16},
    {"i32", TokenType::Int32}, {"i64", TokenType::Int64},
    {"u8", TokenType::UInt8}, {"u16", TokenType::UInt16},
    {"u32", TokenType::UInt32}, {"u64", TokenType::UInt64},
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
  while (!IsEof()) {
    unsigned char c = static_cast<unsigned char>(Peek());
    if (std::isspace(c)) {
      Consume();
    } else if (c == '/' && Peek(1) == '/') {
      while (!IsEof() && Peek() != '\n') Consume();
    } else if (c == '/' && Peek(1) == '*') {
      Consume(); Consume(); // /*
      while (!IsEof() && !(Peek() == '*' && Peek(1) == '/')) Consume();
      if (!IsEof()) { Consume(); Consume(); } // */
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
  if (std::isalpha(c) || c == '_') return ReadIdentifier();
  if (std::isdigit(c)) return ReadNumber();
  if (c == '"') return ReadString();

  common::SourceLocation start_loc = location_;
  Consume();

#define RETURN_TOKEN(token_type) \
  return Token{token_type, std::monostate{}, start_loc}

  switch (c) {
    case ':':
      if (Match(':')) RETURN_TOKEN(TokenType::DoubleColon);
      RETURN_TOKEN(TokenType::Colon);
    case ';': RETURN_TOKEN(TokenType::Semi);
    case ',': RETURN_TOKEN(TokenType::Comma);
    case '?': RETURN_TOKEN(TokenType::Question);
    case '#': RETURN_TOKEN(TokenType::Hash);
    case '$': RETURN_TOKEN(TokenType::Dollar);
    case '@': RETURN_TOKEN(TokenType::At);
    case '~': RETURN_TOKEN(TokenType::Tilda);
    case '^':
      if (Match('=')) RETURN_TOKEN(TokenType::CaretEq);
      RETURN_TOKEN(TokenType::Caret);
    case '.':
      if (Match('.')) {
        if (Match('=')) RETURN_TOKEN(TokenType::DotDotEq);
        RETURN_TOKEN(TokenType::DotDot);
      }
      RETURN_TOKEN(TokenType::Dot);
    case '=':
      if (Match('=')) RETURN_TOKEN(TokenType::EqEq);
      if (Match('>')) RETURN_TOKEN(TokenType::FatArrow);
      RETURN_TOKEN(TokenType::Equal);
    case '+':
      if (Match('=')) RETURN_TOKEN(TokenType::PlusEq);
      RETURN_TOKEN(TokenType::Plus);
    case '-':
      if (Match('>')) RETURN_TOKEN(TokenType::Arrow);
      if (Match('=')) RETURN_TOKEN(TokenType::MinusEq);
      RETURN_TOKEN(TokenType::Minus);
    case '*':
      if (Match('=')) RETURN_TOKEN(TokenType::StarEq);
      RETURN_TOKEN(TokenType::Star);
    case '/':
      if (Match('=')) RETURN_TOKEN(TokenType::SlashEq);
      RETURN_TOKEN(TokenType::Slash);
    case '!':
      if (Match('=')) RETURN_TOKEN(TokenType::NotEq);
      RETURN_TOKEN(TokenType::Exclamation);
    case '&':
      if (Match('&')) RETURN_TOKEN(TokenType::AndAnd);
      if (Match('=')) RETURN_TOKEN(TokenType::AmpersandEq);
      RETURN_TOKEN(TokenType::Ampersand);
    case '|':
      if (Match('|')) RETURN_TOKEN(TokenType::PipePipe);
      if (Match('=')) RETURN_TOKEN(TokenType::PipeEq);
      RETURN_TOKEN(TokenType::Pipe);
    case '<':
      if (Match('<')) {
        if (Match('=')) RETURN_TOKEN(TokenType::LessLessEq);
        RETURN_TOKEN(TokenType::LessLess);
      }
      if (Match('=')) {
        if (Match('>')) RETURN_TOKEN(TokenType::Spaceship);
        RETURN_TOKEN(TokenType::LessEq);
      }
      RETURN_TOKEN(TokenType::Less);
    case '>':
      if (Match('>')) {
        if (Match('=')) RETURN_TOKEN(TokenType::GreaterGreaterEq);
        RETURN_TOKEN(TokenType::GreaterGreater);
      }
      if (Match('=')) RETURN_TOKEN(TokenType::GreaterEq);
      RETURN_TOKEN(TokenType::Greater);
    case '\'':
      return ReadTickOrChar(start_loc);
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

common::Tryable<Token> Lexer::ReadIdentifier() {
  auto start_loc = location_;
  auto start_pos = pos_;
  while (not IsEof() && (std::isalnum(Peek()) || Peek() == '_')) Consume();

  auto text = source_.substr(start_pos, pos_ - start_pos);
  if (text == "_") return Token{TokenType::Underscore, std::monostate{}, start_loc};

  if (auto it = keywords.find(text); it != keywords.end()) {
    if (it->second == TokenType::LiteralBool) {
      return Token{it->second, (text == "true"), start_loc};
    }
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
      if (Peek(1) == '.') break; // range 1..10
      if (is_float) break;
      is_float = true;
      Consume();
    } else break;
  }

  auto num_part = source_.substr(start_pos, pos_ - start_pos);
  std::string_view suffix;
  if (Match('_')) {
    auto s_start = pos_;
    while (not IsEof() && std::isalnum(Peek())) Consume();
    suffix = source_.substr(s_start, pos_ - s_start);
  }
  return FinalizeNumericToken(num_part, suffix, is_float, start_loc);
}

template <typename T>
std::optional<T> ParseString(std::string_view str) {
  T value;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
  return (ec == std::errc{}) ? std::optional<T>(value) : std::nullopt;
}

common::Tryable<Token> Lexer::FinalizeNumericToken(std::string_view str,
                                                   std::string_view suffix,
                                                   bool is_float,
                                                   common::SourceLocation loc) {
  if (is_float) {
    auto val = ParseString<double>(str);
    if (!val) {
      common::DiagnosticEngine::GetInstance().Report(common::DiagnosticLevel::Error, loc, "Invalid float literal");
      return Token{TokenType::Invalid, {}, loc};
    }
    if (suffix == "f32") return Token{TokenType::LiteralFloat, static_cast<float>(*val), loc};
    return Token{TokenType::LiteralFloat, *val, loc};
  }

  std::string_view eff_suffix = suffix;
  if (suffix == "isize") eff_suffix = (sizeof(void*) == 8) ? "i64" : "i32";
  if (suffix == "usize") eff_suffix = (sizeof(void*) == 8) ? "u64" : "u32";

  auto report_overflow = [&]() {
    common::DiagnosticEngine::GetInstance().Report(common::DiagnosticLevel::Error, loc, "Integer overflow");
    return Token{TokenType::Invalid, {}, loc};
  };

  if (eff_suffix == "u8") { if (auto v = ParseString<uint8_t>(str)) return Token{TokenType::LiteralInt, *v, loc}; return report_overflow(); }
  if (eff_suffix == "u16") { if (auto v = ParseString<uint16_t>(str)) return Token{TokenType::LiteralInt, *v, loc}; return report_overflow(); }
  if (eff_suffix == "u32") { if (auto v = ParseString<uint32_t>(str)) return Token{TokenType::LiteralInt, *v, loc}; return report_overflow(); }
  if (eff_suffix == "u64") { if (auto v = ParseString<uint64_t>(str)) return Token{TokenType::LiteralInt, *v, loc}; return report_overflow(); }
  if (eff_suffix == "i8") { if (auto v = ParseString<int8_t>(str)) return Token{TokenType::LiteralInt, *v, loc}; return report_overflow(); }
  if (eff_suffix == "i16") { if (auto v = ParseString<int16_t>(str)) return Token{TokenType::LiteralInt, *v, loc}; return report_overflow(); }
  if (eff_suffix == "i64") { if (auto v = ParseString<int64_t>(str)) return Token{TokenType::LiteralInt, *v, loc}; return report_overflow(); }

  // Default i32
  if (auto v = ParseString<int32_t>(str)) return Token{TokenType::LiteralInt, *v, loc};
  return report_overflow();
}

common::Tryable<Token> Lexer::ReadString() {
  auto start_loc = location_;
  Consume(); // "
  std::string content;
  while (!IsEof() && Peek() != '\"') {
    char c = Consume();
    if (c == '\\') {
      if (IsEof()) break;
      char escaped = Consume();
      switch (escaped) {
        case 'n': content += '\n'; break;
        case 't': content += '\t'; break;
        case 'r': content += '\r'; break;
        case '\\': content += '\\'; break;
        case '\"': content += '\"'; break;
        case '0': content += '\0'; break;
        default:
          common::DiagnosticEngine::GetInstance().Report(common::DiagnosticLevel::Error, start_loc, "Unknown escape");
          return Token{TokenType::Invalid, {}, start_loc};
      }
    } else content += c;
  }
  if (!Match('\"')) {
    common::DiagnosticEngine::GetInstance().Report(common::DiagnosticLevel::Error, start_loc, "Unterminated string");
    return Token{TokenType::Invalid, {}, start_loc};
  }
  return Token{TokenType::LiteralString, std::move(content), start_loc};
}

common::Tryable<Token> Lexer::ReadTickOrChar(common::SourceLocation start_loc) {
  if (IsEof()) return Token{TokenType::Invalid, {}, start_loc};

  if (std::isalpha(Peek()) || Peek() == '_') {
    auto start_pos = pos_;
    while (!IsEof() && (std::isalnum(Peek()) || Peek() == '_')) Consume();
    auto name = source_.substr(start_pos, pos_ - start_pos);

    if (Match('\'')) return Token{TokenType::LiteralChar, static_cast<int32_t>(name[0]), start_loc};
    return Token{TokenType::Tick, std::string(name), start_loc};
  }

  char32_t content;
  if (Match('\\')) {
    if (IsEof()) return Token{TokenType::Invalid, {}, start_loc};
    char escaped = Consume();
    switch (escaped) {
      case 'n': content = '\n'; break;
      case 't': content = '\t'; break;
      case 'r': content = '\r'; break;
      case '\\': content = '\\'; break;
      case '\'': content = '\''; break;
      case '0': content = '\0'; break;
      default: content = escaped;
    }
  } else content = Consume();

  if (!Match('\'')) {
    common::DiagnosticEngine::GetInstance().Report(common::DiagnosticLevel::Error, start_loc, "Unterminated char");
    return Token{TokenType::Invalid, {}, start_loc};
  }
  return Token{TokenType::LiteralChar, static_cast<int32_t>(content), start_loc};
}

} // namespace lexer