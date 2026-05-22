#pragma once

#include <lexer/lexer.hpp>

#include <vector>

#include <memory>

#include "ast/nodes/all.hpp"

#include "precedence.hpp"

namespace parser {

class Parser {
 public:
  explicit Parser(lexer::Lexer& lexer);

  /*
   * main entry point
   */
  std::vector<std::unique_ptr<ast::nodes::Base>> ParseProgram();

 private:
  void Advance();

  [[nodiscard]]
  bool Check(lexer::TokenType type) const;
  bool Match(lexer::TokenType type);
  void Expect(lexer::TokenType type, std::string_view message);
  [[nodiscard]]
  bool IsAtEnd() const;

  /*
   * resynchronizes the parser state after a syntax error to prevent cascading errors
   */
  void Synchronize();

  std::unique_ptr<ast::nodes::Statement> ParseStatement();
  std::unique_ptr<ast::nodes::LetStmt> ParseLet();
  std::unique_ptr<ast::nodes::ReturnStmt> ParseReturn();
  std::unique_ptr<ast::nodes::WhileStmt> ParseWhile();
  std::unique_ptr<ast::nodes::ForInStmt> ParseForIn();
  std::unique_ptr<ast::nodes::BreakStmt> ParseBreak();
  std::unique_ptr<ast::nodes::ContinueStmt> ParseContinue();
  std::unique_ptr<ast::nodes::DeferStmt> ParseDefer();
  std::unique_ptr<ast::nodes::StaticIfStmt> ParseStaticIf();
  std::unique_ptr<ast::nodes::Statement> ParseAssignOrExprStmt();

  std::unique_ptr<ast::nodes::FuncDecl> ParseFuncDecl();
  std::unique_ptr<ast::nodes::StructDecl> ParseStructDecl();
  std::unique_ptr<ast::nodes::EnumDecl> ParseEnumDecl();
  std::unique_ptr<ast::nodes::TraitDecl> ParseTraitDecl();
  std::unique_ptr<ast::nodes::ImplDecl> ParseImplDecl();
  std::unique_ptr<ast::nodes::TypeAliasDecl> ParseTypeAlias();

  std::unique_ptr<ast::nodes::Base> ParseTopLevelItem();

  std::unique_ptr<ast::nodes::ModuleDecl> ParseModuleDecl();
  std::unique_ptr<ast::nodes::ImportDecl> ParseImportDecl();
  std::unique_ptr<ast::nodes::ExportDecl> ParseExportDecl();

  std::unique_ptr<ast::nodes::Expression> ParseExpression(Precedence precedence = Precedence::LOWEST);
  std::unique_ptr<ast::nodes::Expression> ParsePrefix();
  std::unique_ptr<ast::nodes::Expression> ParseInfix(std::unique_ptr<ast::nodes::Expression> left);

  std::unique_ptr<ast::nodes::BlockExpr> ParseBlockExpr();
  std::unique_ptr<ast::nodes::IfExpr> ParseIfExpr();
  std::unique_ptr<ast::nodes::MatchExpr> ParseMatchExpr();
  std::unique_ptr<ast::nodes::LoopExpr> ParseLoopExpr();
  std::unique_ptr<ast::nodes::LambdaExpr> ParseLambdaExpr();
  std::unique_ptr<ast::nodes::NewExpr> ParseNewExpr();

  std::unique_ptr<ast::nodes::Type> ParseType();
  std::unique_ptr<ast::nodes::Path> ParsePath();
  std::unique_ptr<ast::nodes::Pattern> ParsePattern();

  std::vector<std::unique_ptr<ast::nodes::Attribute>> ParseAttributes();
  std::vector<std::unique_ptr<ast::nodes::GenericParameter>> ParseGenericParameters();
  std::vector<std::unique_ptr<ast::nodes::WherePredicate>> ParseWhereClause();

  [[nodiscard]]
  Precedence GetCurrentPrecedence() const;

  [[nodiscard]]
  bool IsLiteral(lexer::TokenType type) const;

  [[nodiscard]]
  bool IsPrimitiveType(lexer::TokenType type) const;

 private:
  lexer::Lexer& lexer_;
  lexer::Token current_token_;
  lexer::Token previous_token_;
};;

}  // namespace parser