#pragma once

#include <variant>
#include <string>

#include <cstdint>

#include <common/source_location.hpp>
#include <common/internal_error.hpp>

namespace lexer {

enum class TokenType {
  // keywords
  Let, Mut, Const, Volatile, Static, As, Func, Inline, Lazy, Return,
  Trait, Impl, Where, Requires, // for traits
  If, Else, While, Loop, For, In, Break, Continue, StaticIf,
  Module, Export, Import, Extern,
  Using,
  Self,
  Enum, Match, At, Struct, Ref,
  // metadata
  Sizeof, AlignOf, Decltype, Reflect,
  // visibility
  Public, Private, Protected,
  Safe, Unsafe,
  // concurrency
  Async, Await,
  // heap
  Dyn, New, Box,
  // for lambdas
  Move,

  // primitive types
  Int8, Int16, Int32, Int64, Int128,
  UInt8, UInt16, UInt32, UInt64, UInt128,
  Float32, Float64, Bool, String, Unit,
  ISize, USize,

  // identifiers & literals
  Identifier,
  LiteralInt, LiteralFloat,
  LiteralChar, LiteralString,
  LiteralBool,

  // punctuation
  Underscore, // _
  Colon,      // :
  DoubleColon,// ::
  Semi,       // ;
  Comma,      // ,
  Dot,        // .
  Arrow,      // -> (ReturnType)
  FatArrow,   // => (Match/Requires)
  Question,   // ?
  Hash,       // # (for metadata)
  Dollar,     // $
  Tick,       // ' (lifetimes)

  // math, logic, bitwise
  Equal,      // =
  EqEq,       // ==
  NotEq,      // !=
  Spaceship,
  Plus, Minus, Star, Slash,
  PlusEq, MinusEq, StarEq, SlashEq,
  Ampersand, Pipe, Caret, Tilda,
  AmpersandEq, PipeEq, CaretEq,
  AndAnd, PipePipe, Exclamation,
  LessLess, GreaterGreater,
  LessLessEq, GreaterGreaterEq,
  LessEq, GreaterEq,

  // syntax sugar
  LogicAnd, LogicOr, LogicNot, // 'not', 'and', 'or'

  // ranges
  DotDot,
  DotDotEq,

  // generics & delimiters
  Less,       // <  (and generic opening)
  Greater,    // >  (and generic ending)
  LParen, RParen, // ()
  LBrace, RBrace, // {}
  LBracket, RBracket, // []

  Eof, Invalid
};

using TokenData = std::variant<
    std::monostate, // keywords, punctuation and errors
    std::int8_t,    // int8
    std::uint8_t,   // uint8
    std::int16_t,   // unt16
    std::uint16_t,  // uint16
    std::int32_t,   // int32, char
    std::uint32_t,  // uint32
    std::int64_t,   // int64
    std::uint64_t,  // uint64
    __int128_t,     // int128
    __uint128_t,    // uint128
    float,          // float32
    double,         // float64
    std::string,    // identifiers, string literals
    bool            // bool
>;

struct Token {
  template<typename T>
  [[nodiscard]]
  bool Is() const {
    return std::holds_alternative<T>(data);
  }

  template<typename T>
  [[nodiscard]]
  common::Tryable<std::reference_wrapper<const T>> As() const {
    const T* ptr = std::get_if<T>(&data);

    if (ptr) {
      return std::cref(*ptr);
    }

    return MAKE_INTERNAL_ERROR(
        common::InternalErrorKind::InvariantViolated,
        "attempted to access TokenData as " + std::string(typeid(T).name()) +
        ", but it holds a different type."
    );
  }

  TokenType type;
  TokenData data;
  common::SourceLocation loc;
};

}  // namespace lexer