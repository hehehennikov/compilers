#include <gtest/gtest.h>
#include <sstream>
#include <string_view>
#include <vector>

#include "parser/parser.hpp"

#include "common/diagnostic.hpp"
#include "lexer/lexer.hpp"
#include <parser/ast/visitor/print.hpp>
#include <parser/ast/nodes/all.hpp>

namespace parser::test {

class ParserTest : public ::testing::Test {
 protected:
  std::vector<std::unique_ptr<ast::nodes::Base>> Parse(std::string_view source) {
    lexer::Lexer lexer{source, "test_input"};
    Parser p{lexer};
    return p.ParseProgram();
  }

  std::string GetAstString(const std::vector<std::unique_ptr<ast::nodes::Base>>& nodes) {
    std::stringstream ss;
    ast::visitor::Print printer{ss};
    
    for (const auto& node : nodes) {
      if (node != nullptr) {
        node->Accept(&printer);
      }
    }
    
    return ss.str();
  }
};

TEST_F(ParserTest, ParseIntegerLiteral) {
  const auto nodes = Parse("42");
  
  ASSERT_EQ(nodes.size(), 1);
  EXPECT_NE(nodes[0], nullptr);
  
  const auto output = GetAstString(nodes);
  EXPECT_TRUE(output.contains("LiteralExpr: 42"));
}

TEST_F(ParserTest, ParseStringLiteral) {
  const auto nodes = Parse("\"compiler\"");
  
  ASSERT_EQ(nodes.size(), 1);
  
  const auto output = GetAstString(nodes);
  EXPECT_TRUE(output.contains("LiteralExpr: \"compiler\""));
}

TEST_F(ParserTest, ParseBooleanLiterals) {
  const auto nodes = Parse("true false");
  
  ASSERT_EQ(nodes.size(), 2);
  
  const auto output = GetAstString(nodes);
  EXPECT_TRUE(output.contains("LiteralExpr: true"));
  EXPECT_TRUE(output.contains("LiteralExpr: false"));
}

TEST_F(ParserTest, ParseIdentifiers) {
  const auto nodes = Parse("variable_name myVar_123");
  
  ASSERT_EQ(nodes.size(), 2);
  
  const auto output = GetAstString(nodes);
  EXPECT_TRUE(output.contains("IdentExpr: variable_name"));
  EXPECT_TRUE(output.contains("IdentExpr: myVar_123"));
}

TEST_F(ParserTest, ParseGroupedExpression) {
  const auto nodes = Parse("(42)");
  
  ASSERT_EQ(nodes.size(), 1);
  
  const auto output = GetAstString(nodes);
  EXPECT_TRUE(output.contains("LiteralExpr: 42"));
}

TEST_F(ParserTest, ParseUnaryExpression) {
  const auto nodes = Parse("-42");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  // UnaryExpr (op: Minus) -> LiteralExpr: 42
  EXPECT_TRUE(output.contains("UnaryExpr"));
  EXPECT_TRUE(output.contains("42"));
}

TEST_F(ParserTest, ParseBinaryExpressionPrecedence) {
  // 1 + (2 * 3)
  const auto nodes = Parse("1 + 2 * 3");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("BinaryExpr"));
}

TEST_F(ParserTest, ParseComplexExpression) {
  // -(a + b) * &c
  const auto nodes = Parse("-(a + b) * &c");

  ASSERT_EQ(nodes.size(), 1);
  EXPECT_NE(nodes[0], nullptr);

  const auto output = GetAstString(nodes);
  EXPECT_TRUE(output.contains("UnaryExpr"));
  EXPECT_TRUE(output.contains("BinaryExpr"));
}

TEST_F(ParserTest, ParseTypeCasting) {
  const auto nodes = Parse("x as f64");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("CastExpr (as)"));
  EXPECT_TRUE(output.contains("Type: Primitive"));
}

TEST_F(ParserTest, ParseFunctionCall) {
  const auto nodes = Parse("foo(1, a + b)");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("CallExpr"));
  EXPECT_TRUE(output.contains("Arguments (2)"));
}

TEST_F(ParserTest, ParseMemberAccess) {
  const auto nodes = Parse("point.x + point.move(1, 2)");

  const auto output = GetAstString(nodes);
  ASSERT_EQ(nodes.size(), 1);

  EXPECT_TRUE(output.contains("MemberAccessExpr (.x)"));
  EXPECT_TRUE(output.contains("MethodCallExpr (.move)"));
}

TEST_F(ParserTest, ParseGenericType) {
  const auto nodes = Parse("v as Vec<i32>");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("Type: Generic Application"));
  EXPECT_TRUE(output.contains("Template: Vec"));
  EXPECT_TRUE(output.contains("Type: Primitive"));
}

TEST_F(ParserTest, ParseSimpleLet) {
  const auto nodes = Parse("let x = 42;");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("LetStmt"));
  EXPECT_TRUE(output.contains("Pattern: Binding"));
  EXPECT_TRUE(output.contains("name: \"x\", mut: false"));
  EXPECT_TRUE(output.contains("42"));
}

TEST_F(ParserTest, ParseMutableLetWithType) {
  const auto nodes = Parse("let mut y: i32 = 100;");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("LetStmt (mutable: true)"));
  EXPECT_TRUE(output.contains("Type: Primitive"));
  EXPECT_TRUE(output.contains("token_kind: 49"));
}

TEST_F(ParserTest, ParseWildcardLet) {
  const auto nodes = Parse("let _ = foo();");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("Pattern: Wildcard (_)"));
  EXPECT_TRUE(output.contains("CallExpr"));
}

TEST_F(ParserTest, ParseAssignment) {
  const auto nodes = Parse("x = x + 1;");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("AssignStmt"));
  EXPECT_TRUE(output.contains("BinaryExpr"));
}

TEST_F(ParserTest, ParseWhileLoop) {
  const auto nodes = Parse("while x > 0 { x = x - 1; }");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  std::cout << output << '\n';

  EXPECT_TRUE(output.contains("WhileStmt"));
  EXPECT_TRUE(output.contains("Greater"));
}

TEST_F(ParserTest, ParseIfExpression) {
  const auto nodes = Parse("let val = if c { 1 } else { 0 };");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("IfExpr"));
  EXPECT_TRUE(output.contains("LiteralExpr: 1"));
  EXPECT_TRUE(output.contains("LiteralExpr: 0"));
}

TEST_F(ParserTest, ParseReturn) {
  const auto nodes = Parse("return 42;");

  ASSERT_EQ(nodes.size(), 1);
  const auto output = GetAstString(nodes);

  EXPECT_TRUE(output.contains("ReturnStmt"));
  EXPECT_TRUE(output.contains("42"));
}

}  // namespace parser::test




int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}