#include <gtest/gtest.h>

#include <vector>

#include <lexer/lexer.hpp>

#include <common/diagnostic.hpp>

namespace lexer {

class LexerTest : public ::testing::Test {
 protected:
  std::vector<Token> Tokenize(std::string_view source) {
    Lexer lexer(source, "test_input");
    std::vector<Token> tokens;
    while (true) {
      auto result = lexer.NextToken();
      if (not result) {
        throw std::runtime_error("Internal lexer error during test");
      }
      tokens.push_back(std::move(*result));
      if (tokens.back().type == TokenType::Eof) {
        break;
      }
    }
    return tokens;
  }
};

TEST_F(LexerTest, Keywords) {
  auto tokens = Tokenize("let mut func return safe");

  ASSERT_EQ(tokens.size(), 6);
  EXPECT_EQ(tokens[0].type, TokenType::Let);
  EXPECT_EQ(tokens[1].type, TokenType::Mut);
  EXPECT_EQ(tokens[2].type, TokenType::Func);
  EXPECT_EQ(tokens[3].type, TokenType::Return);
  EXPECT_EQ(tokens[4].type, TokenType::Safe);
}

TEST_F(LexerTest, NumericLiterals) {
  auto tokens = Tokenize("42 100_u64 3.14_f32 100_i8");

  EXPECT_EQ(tokens[0].type, TokenType::LiteralInt);
  EXPECT_EQ(tokens[0].As<std::int32_t>().value().get(), 42);

  EXPECT_EQ(tokens[1].type, TokenType::LiteralInt);
  EXPECT_EQ(tokens[1].As<std::uint64_t>().value().get(), 100);

  EXPECT_EQ(tokens[2].type, TokenType::LiteralFloat);
  EXPECT_FLOAT_EQ(tokens[2].As<float>().value().get(), 3.14f);

  EXPECT_EQ(tokens[3].type, TokenType::LiteralInt);
  EXPECT_FLOAT_EQ(tokens[3].As<std::int8_t>().value().get(), 100);
}

TEST_F(LexerTest, ComplexOperators) {
  auto tokens = Tokenize("<<= <=> -> ..= ?");

  EXPECT_EQ(tokens[0].type, TokenType::LessLessEq);
  EXPECT_EQ(tokens[1].type, TokenType::Spaceship);
  EXPECT_EQ(tokens[2].type, TokenType::Arrow);
  EXPECT_EQ(tokens[3].type, TokenType::DotDotEq);
  EXPECT_EQ(tokens[4].type, TokenType::Question);
}

TEST_F(LexerTest, RangeEdgeCase) {
  auto tokens = Tokenize("1..10");

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].type, TokenType::LiteralInt);
  EXPECT_EQ(tokens[1].type, TokenType::DotDot);
  EXPECT_EQ(tokens[2].type, TokenType::LiteralInt);
}

TEST_F(LexerTest, StringEscaping) {
  auto tokens = Tokenize(R"("hello\nworld")");

  EXPECT_EQ(tokens[0].type, TokenType::LiteralString);
  EXPECT_EQ(tokens[0].As<std::string>().value().get(), "hello\nworld");
}

TEST_F(LexerTest, TickAndChar) {
  auto tokens = Tokenize("'a 'static 'c'");

  EXPECT_EQ(tokens[0].type, TokenType::Tick);
  EXPECT_EQ(tokens[0].As<std::string>().value().get(), "a");

  EXPECT_EQ(tokens[1].type, TokenType::Tick);
  EXPECT_EQ(tokens[1].As<std::string>().value().get(), "static");

  EXPECT_EQ(tokens[2].type, TokenType::LiteralChar);
  EXPECT_EQ(tokens[2].As<int32_t>().value().get(), 'c');
}

TEST_F(LexerTest, Comments) {
  auto tokens = Tokenize(
    "let // comment\n"
    "x /* block */ = 5;");

  ASSERT_EQ(tokens.size(), 6); // let, x, =, 5, ;, Eof
  EXPECT_EQ(tokens[0].type, TokenType::Let);
  EXPECT_EQ(tokens[1].type, TokenType::Identifier);
}

TEST_F(LexerTest, UnknownCharacter) {
  common::DiagnosticEngine::GetInstance().Flush();

  auto tokens = Tokenize("let mut x = `;"); // ` is unknown

  auto x = tokens.begin() - tokens.end();

  EXPECT_TRUE(common::DiagnosticEngine::GetInstance().HasErrors());
  EXPECT_EQ(tokens[4].type, TokenType::Invalid);
  common::DiagnosticEngine::GetInstance().Clear();
}

TEST_F(LexerTest, Func) {
  common::DiagnosticEngine::GetInstance().Flush();

  auto tokens = Tokenize(
    "func foo(x : &i32, y : &f64) -> unit {\n"
          "  x = 42;\n"
          "  y = 3.14;\n"
          "  return ();\n"
          "}");

  ASSERT_EQ(tokens.size(), 30);
  EXPECT_EQ(tokens[0].type, TokenType::Func);
  EXPECT_EQ(tokens[3].type, TokenType::Identifier);
  EXPECT_EQ(tokens[4].type, TokenType::Colon);
  EXPECT_EQ(tokens[5].type, TokenType::Ampersand);
  EXPECT_EQ(tokens[6].type, TokenType::Int32);
  EXPECT_EQ(tokens[7].type, TokenType::Comma);
  EXPECT_EQ(tokens[8].type, TokenType::Identifier);
  EXPECT_EQ(tokens[9].type, TokenType::Colon);
  EXPECT_EQ(tokens[10].type, TokenType::Ampersand);
  EXPECT_EQ(tokens[11].type, TokenType::Float64);
}

} // namespace lexer

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}