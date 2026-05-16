#pragma once

#include <unordered_map>

#include <lexer/token.hpp>

namespace parser {

enum class Precedence {
  LOWEST,
  LOGIC,      // and, or
  COMPARISON, // ==, !=, <, >
  SUM,        // +, -
  PRODUCT,    // *, /
  CAST,       // as
  PREFIX,     // -x, !x
  CALL,       // foo(), obj.prop, arr[i]
};

inline const  std::unordered_map<lexer::TokenType, Precedence> Precedences = {
  {lexer::TokenType::LogicAnd, Precedence::LOGIC},
  {lexer::TokenType::LogicOr, Precedence::LOGIC},
  {lexer::TokenType::EqEq, Precedence::COMPARISON},
  {lexer::TokenType::NotEq, Precedence::COMPARISON},
  {lexer::TokenType::Less, Precedence::COMPARISON},
  {lexer::TokenType::Greater, Precedence::COMPARISON},
  {lexer::TokenType::Plus, Precedence::SUM},
  {lexer::TokenType::Minus, Precedence::SUM},
  {lexer::TokenType::Star, Precedence::PRODUCT},
  {lexer::TokenType::Slash, Precedence::PRODUCT},
  {lexer::TokenType::As, Precedence::CAST},
  {lexer::TokenType::LParen, Precedence::CALL},
  {lexer::TokenType::Dot, Precedence::CALL},
  {lexer::TokenType::LBracket, Precedence::CALL}
};

}  // namespace parser