#pragma once

#include "visitor.hpp"

#include <lexer/lexer.hpp>

#include <iostream>

namespace parser::ast::visitor {

class Print : public IVisitor {
 public:
  explicit Print(std::ostream& out = std::cout);

 public:
  void Run(nodes::Base* node);

 public:
  void Visit(nodes::ModuleDecl* node) override;
  void Visit(nodes::ImportDecl* node) override;
  void Visit(nodes::ExportDecl* node) override;
  void Visit(nodes::ExternDecl* node) override;
  void Visit(nodes::Visibility* node) override;

  void Visit(nodes::Attribute* node) override;
  void Visit(nodes::Path* node) override;
  void Visit(nodes::Lifetime* node) override;
  void Visit(nodes::GenericParameter* node) override;
  void Visit(nodes::RequiresClause* node) override;
  void Visit(nodes::WherePredicate* node) override;

  void Visit(nodes::LiteralExpr* node) override;
  void Visit(nodes::IdentExpr* node) override;
  void Visit(nodes::SelfExpr* node) override;
  void Visit(nodes::BorrowExpr* node) override;
  void Visit(nodes::UnitExpr* node) override;

  void Visit(nodes::UnaryExpr* node) override;
  void Visit(nodes::BinaryExpr* node) override;
  void Visit(nodes::SpaceshipExpr* node) override;
  void Visit(nodes::CastExpr* node) override;

  void Visit(nodes::CallExpr* node) override;
  void Visit(nodes::MethodCallExpr* node) override;
  void Visit(nodes::MemberAccessExpr* node) override;
  void Visit(nodes::IndexAccessExpr* node) override;

  void Visit(nodes::BlockExpr* node) override;
  void Visit(nodes::IfExpr* node) override;
  void Visit(nodes::MatchExpr* node) override;
  void Visit(nodes::LoopExpr* node) override;
  void Visit(nodes::TryExpr* node) override;

  void Visit(nodes::NewExpr* node) override;
  void Visit(nodes::LambdaExpr* node) override;

  void Visit(nodes::SizeofExpr* node) override;
  void Visit(nodes::AlignofExpr* node) override;
  void Visit(nodes::DecltypeExpr* node) override;
  void Visit(nodes::ReflectExpr* node) override;

  void Visit(nodes::ExprStmt* node) override;
  void Visit(nodes::LetStmt* node) override;
  void Visit(nodes::AssignStmt* node) override;
  void Visit(nodes::WhileStmt* node) override;
  void Visit(nodes::ForInStmt* node) override;
  void Visit(nodes::BreakStmt* node) override;
  void Visit(nodes::ContinueStmt* node) override;
  void Visit(nodes::ReturnStmt* node) override;
  void Visit(nodes::DeferStmt* node) override;
  void Visit(nodes::StaticIfStmt* node) override;

  void Visit(nodes::FuncDecl* node) override;
  void Visit(nodes::StructDecl* node) override;
  void Visit(nodes::EnumDecl* node) override;
  void Visit(nodes::ImplDecl* node) override;
  void Visit(nodes::TraitDecl* node) override;
  void Visit(nodes::TypeAliasDecl* node) override;

  void Visit(nodes::PrimitiveType* node) override;
  void Visit(nodes::ReferenceType* node) override;
  void Visit(nodes::PointerType* node) override;
  void Visit(nodes::ArrayType* node) override;
  void Visit(nodes::GenericType* node) override;
  void Visit(nodes::FunctionType* node) override;
  void Visit(nodes::PlaceholderType* node) override;

  void Visit(nodes::WildcardPattern* node) override;
  void Visit(nodes::BindingPattern* node) override;
  void Visit(nodes::LiteralPattern* node) override;
  void Visit(nodes::RangePattern* node) override;
  void Visit(nodes::TuplePattern* node) override;
  void Visit(nodes::StructPattern* node) override;
  void Visit(nodes::ReferencePattern* node) override;

 private:
  void Indent() const;
  void VisitChild(nodes::Base* node);
  void PrintValue(const lexer::TokenData& data) const;
  void PrintLocation(const common::SourceLocation& loc) const;

 private:
  std::ostream& out_;
  std::uint32_t indent_level_ = 0;
};

}  // namespace parser::ast::visitor