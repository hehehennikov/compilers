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
  ASSIGN,
  LOGIC_OR,
  LOGIC_AND,
  RANGE,
};

inline const std::unordered_map<lexer::TokenType, Precedence> Precedences = {
    // Assignments
    {lexer::TokenType::PlusEq,           Precedence::ASSIGN},
    {lexer::TokenType::MinusEq,          Precedence::ASSIGN},
    {lexer::TokenType::StarEq,           Precedence::ASSIGN},
    {lexer::TokenType::SlashEq,          Precedence::ASSIGN},
    {lexer::TokenType::AmpersandEq,      Precedence::ASSIGN},
    {lexer::TokenType::PipeEq,           Precedence::ASSIGN},
    {lexer::TokenType::CaretEq,          Precedence::ASSIGN},
    {lexer::TokenType::LessLessEq,       Precedence::ASSIGN},
    {lexer::TokenType::GreaterGreaterEq, Precedence::ASSIGN},

    // Logic
    {lexer::TokenType::LogicOr,          Precedence::LOGIC_OR},
    {lexer::TokenType::PipePipe,         Precedence::LOGIC_OR},
    {lexer::TokenType::LogicAnd,         Precedence::LOGIC_AND},
    {lexer::TokenType::AndAnd,           Precedence::LOGIC_AND},

    // Comparison
    {lexer::TokenType::EqEq,             Precedence::COMPARISON},
    {lexer::TokenType::NotEq,            Precedence::COMPARISON},
    {lexer::TokenType::Less,             Precedence::COMPARISON},
    {lexer::TokenType::Greater,          Precedence::COMPARISON},
    {lexer::TokenType::LessEq,           Precedence::COMPARISON},
    {lexer::TokenType::GreaterEq,        Precedence::COMPARISON},
    {lexer::TokenType::Spaceship,        Precedence::COMPARISON},

    // Ranges
    {lexer::TokenType::DotDot,           Precedence::RANGE},
    {lexer::TokenType::DotDotEq,         Precedence::RANGE},

    // Arithmetic & Bitwise Sum
    {lexer::TokenType::Plus,             Precedence::SUM},
    {lexer::TokenType::Minus,            Precedence::SUM},
    {lexer::TokenType::Pipe,             Precedence::SUM},
    {lexer::TokenType::Caret,            Precedence::SUM},

    // Arithmetic & Bitwise Product
    {lexer::TokenType::Star,             Precedence::PRODUCT},
    {lexer::TokenType::Slash,            Precedence::PRODUCT},
    {lexer::TokenType::Ampersand,        Precedence::PRODUCT},
    {lexer::TokenType::LessLess,         Precedence::PRODUCT},
    {lexer::TokenType::GreaterGreater,   Precedence::PRODUCT},

    // Cast
    {lexer::TokenType::As,               Precedence::CAST},

    // Call, Access & Postfix
    {lexer::TokenType::LParen,           Precedence::CALL},
    {lexer::TokenType::Dot,              Precedence::CALL},
    {lexer::TokenType::LBracket,         Precedence::CALL},
    {lexer::TokenType::Question,         Precedence::CALL}
};
;

}  // namespace parser