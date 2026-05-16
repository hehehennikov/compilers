#pragma once

#include <lexer/lexer.hpp>

#include <vector>

#include <memory>

#include "ast/nodes/fwd.hpp"

#include "precedence.hpp"

namespace parser {

class Parser {
 public:
  explicit Parser(lexer::Lexer& lexer);

 public:
  std::vector<std::unique_ptr<ast::nodes::Statement>> ParseProgram();

 private:
  // cursor
  void Advance();

  bool Check(lexer::TokenType type) const;

  bool Match(lexer::TokenType type);

  void Expect(lexer::TokenType type, std::string_view message);

  Precedence GetCurrentPrecedence();

  bool IsAtEnd() const;

  std::unique_ptr<ast::nodes::Statement> ParseStatement();
  std::unique_ptr<ast::nodes::LetStmt> ParseLet();
  std::unique_ptr<ast::nodes::ReturnStmt> ParseReturn();
  std::unique_ptr<ast::nodes::WhileStmt> ParseWhile();
  std::unique_ptr<ast::nodes::LoopStmt> ParseLoop();
  std::unique_ptr<ast::nodes::ForInStmt> ParseForIn();
  std::unique_ptr<ast::nodes::BreakStmt> ParseBreak();
  std::unique_ptr<ast::nodes::ContinueStmt> ParseContinue();
  std::unique_ptr<ast::nodes::Statement> ParseStaticIf();
  std::unique_ptr<ast::nodes::Statement> ParseAssignOrExprStmt();

  // decls
  std::unique_ptr<ast::nodes::Declaration> ParseDeclaration();
  std::unique_ptr<ast::nodes::FuncDecl> ParseFuncDecl();
  std::unique_ptr<ast::nodes::StructDecl> ParseStructDecl();
  std::unique_ptr<ast::nodes::EnumDecl> ParseEnumDecl();
  std::unique_ptr<ast::nodes::TraitDecl> ParseTraitDecl();
  std::unique_ptr<ast::nodes::ImplDecl> ParseImplDecl();
  std::unique_ptr<ast::nodes::TypeAliasDecl> ParseTypeAlias();

  // expr
  std::unique_ptr<ast::nodes::Expression> ParseExpression(Precedence precedence = Precedence::LOWEST);

  std::unique_ptr<ast::nodes::Expression> ParsePrefix();

  std::unique_ptr<ast::nodes::Expression> ParseInfix(std::unique_ptr<ast::nodes::Expression> left);

  std::unique_ptr<ast::nodes::BlockExpr> ParseBlockExpr();
  std::unique_ptr<ast::nodes::IfExpr> ParseIfExpr();
  std::unique_ptr<ast::nodes::MatchExpr> ParseMatchExpr();
  std::unique_ptr<ast::nodes::NewExpr> ParseNewExpr();
  std::unique_ptr<ast::nodes::CallExpr> ParseCallArguments(std::unique_ptr<ast::nodes::Expression> callee);
  std::unique_ptr<ast::nodes::CastExpr> ParseCastExpr(std::unique_ptr<ast::nodes::Expression> left);

  // types
  std::unique_ptr<ast::nodes::TypeNode> ParseType();
  std::unique_ptr<ast::nodes::PrimitiveType> ParsePrimitiveType();
  std::unique_ptr<ast::nodes::ReferenceType> ParseReferenceType();
  std::unique_ptr<ast::nodes::PointerType> ParsePointerType();
  std::unique_ptr<ast::nodes::GenericType> ParseGenericType();
  std::unique_ptr<ast::nodes::ArrayType> ParseArrayType();
  std::unique_ptr<ast::nodes::FunctionType> ParseFunctionType();

  // patterns
  std::unique_ptr<ast::nodes::PatternNode> ParsePattern();
  std::unique_ptr<ast::nodes::TuplePattern> ParseTuplePattern();
  std::unique_ptr<ast::nodes::StructPattern> ParseStructPattern();

  std::vector<std::unique_ptr<ast::nodes::Attribute>> ParseAttributes();
  std::unique_ptr<ast::nodes::Attribute> ParseSingleAttribute();

  std::unique_ptr<ast::nodes::Lifetime> ParseLifetime();

  std::vector<std::unique_ptr<ast::nodes::GenericParameter>> ParseGenericParameters();
  std::vector<std::unique_ptr<ast::nodes::WherePredicate>> ParseWhereClause();

  bool IsLiteral(lexer::TokenType type) const;
  bool IsPrimitiveType(lexer::TokenType type) const;

 private:
  lexer::Lexer& lexer_;
  lexer::Token cur_tok_;
  lexer::Token prev_tok_;
};

}  // namespace parser