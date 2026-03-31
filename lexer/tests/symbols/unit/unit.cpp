#include <gtest/gtest.h>

#include <fstream>

#include <user.hpp>

TEST(SemanticAnalysis, DoubleDeclarationError) {
  std::string code = R"(
    var x: int;
    var x: int;
  )";
  auto result = RunCompiler(code);
  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(result.error().message.find("already declared") != std::string::npos);
}

TEST(SemanticAnalysis, UndeclaredVariableError) {
  std::string code = R"(
    x = 10;
  )";
  auto result = RunCompiler(code);
  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(result.error().message.find("undeclared") != std::string::npos);
}

TEST(SemanticAnalysis, ValidShadowing) {
  std::string code = R"(
    var x: int;
    x = 10;
    if (x == 10) {
      var x: int;
      x = 5;
    }
  )";

  auto result = RunCompiler(code);
  EXPECT_TRUE(result.has_value());
}

TEST(SemanticAnalysis, UninitializedVariableError) {
  std::string code = R"(
    var x: int;
    print(x);
  )";
  auto result = RunCompiler(code);
  EXPECT_FALSE(result.has_value());
  EXPECT_TRUE(result.error().message.find("used before being initialized") != std::string::npos);
}

TEST(SemanticAnalysis, ShadowingInterpretationCorrectness) {
  std::string code = R"(
    var x: int;
    x = 10;
    if (x == 10) {
      var x: int;
      x = 42;
      print(x);
    }
    print(x);
  )";

  std::stringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

  auto result = RunCompiler(code);

  std::cout.rdbuf(old);

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(buffer.str(), "42\n10\n");
}

TEST(SemanticAnalysis, ParentScopeAccess) {
  std::string code = R"(
    var x: int;
    x = 10;
    if (x == 10) {
      x = 99;
    }
    print(x);
  )";

  std::stringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

  auto result = RunCompiler(code);
  std::cout.rdbuf(old);

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(buffer.str(), "99\n");
}