#include <gtest/gtest.h>

#include <fstream>

#include <user.hpp>

std::unique_ptr<MainFunction> Compile(std::string_view code) {
  auto tokens = Lexer(code).Tokenize();
  if (not tokens) {
    return nullptr;
  }
  auto program = Parser(std::move(*tokens)).Parse();
  if (not program) {
    return nullptr;
  }

  return std::move(*program);
}

TEST(VisitorTests, PrintVisitorTest) {
  std::string code = R"(
    var x: int;
    x = 10;
    while (x == 10) {
      print(x);
    }
  )";

  auto ast = Compile(code);
  ASSERT_NE(ast, nullptr);

  const std::string filename = "ast_output.txt";
  {
    PrintVisitor printer(filename);
    ast->Accept(&printer);
  }

  std::ifstream infile(filename);
  std::string content((std::istreambuf_iterator<char>(infile)),
                       std::istreambuf_iterator<char>());
  
  EXPECT_TRUE(content.find("MainFunction:") != std::string::npos);
  EXPECT_TRUE(content.find("Declare: x") != std::string::npos);
  EXPECT_TRUE(content.find("While: BinaryExpr(Variable(x) == Literal(10))") != std::string::npos);
}

TEST(VisitorTests, InterpreterWhileExecution) {
  std::string code = R"(
    var counter: int;
    var flag: int;
    counter = 0;
    flag = 0;
    while (flag == 0) {
      counter = 1;
      flag = 1;
    }
  )";

  auto ast = Compile(code);
  ASSERT_NE(ast, nullptr);

  Interpreter interpreter;
  EXPECT_NO_THROW(ast->Accept(&interpreter));
}

TEST(VisitorTests, InterpreterPrintCapture) {
  std::string code = R"(
    var x: int;
    x = 42;
    print(x);
  )";

  auto ast = Compile(code);
  ASSERT_NE(ast, nullptr);

  std::stringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

  Interpreter interpreter;
  ast->Accept(&interpreter);

  std::cout.rdbuf(old);
  EXPECT_EQ(buffer.str(), "42\n");
}