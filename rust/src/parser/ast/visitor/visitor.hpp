#pragma once

#include <parser/ast/nodes/fwd.hpp>

namespace parser::ast::visitor {

class IVisitor {
 public:
  virtual ~IVisitor() = default;

 public:
  virtual void Visit(nodes::ModuleDecl* node) = 0;
  virtual void Visit(nodes::ImportDecl* node) = 0;
  virtual void Visit(nodes::ExportDecl* node) = 0;
  virtual void Visit(nodes::ExternDecl* node) = 0;
  virtual void Visit(nodes::Visibility* node) = 0;

  virtual void Visit(nodes::Attribute* node) = 0;
  virtual void Visit(nodes::Path* node) = 0;
  virtual void Visit(nodes::Lifetime* node) = 0;
  virtual void Visit(nodes::GenericParameter* node) = 0;
  virtual void Visit(nodes::RequiresClause* node) = 0;
  virtual void Visit(nodes::WherePredicate* node) = 0;

  virtual void Visit(nodes::LiteralExpr* node) = 0;
  virtual void Visit(nodes::IdentExpr* node) = 0;
  virtual void Visit(nodes::SelfExpr* node) = 0;
  virtual void Visit(nodes::BorrowExpr* node) = 0;
  virtual void Visit(nodes::UnitExpr* node) = 0;

  virtual void Visit(nodes::UnaryExpr* node) = 0;
  virtual void Visit(nodes::BinaryExpr* node) = 0;
  virtual void Visit(nodes::SpaceshipExpr* node) = 0;
  virtual void Visit(nodes::CastExpr* node) = 0;

  virtual void Visit(nodes::CallExpr* node) = 0;
  virtual void Visit(nodes::MethodCallExpr* node) = 0;
  virtual void Visit(nodes::MemberAccessExpr* node) = 0;
  virtual void Visit(nodes::IndexAccessExpr* node) = 0;

  virtual void Visit(nodes::BlockExpr* node) = 0;
  virtual void Visit(nodes::IfExpr* node) = 0;
  virtual void Visit(nodes::MatchExpr* node) = 0;
  virtual void Visit(nodes::LoopExpr* node) = 0;
  virtual void Visit(nodes::TryExpr* node) = 0;

  virtual void Visit(nodes::NewExpr* node) = 0;
  virtual void Visit(nodes::LambdaExpr* node) = 0;

  virtual void Visit(nodes::SizeofExpr* node) = 0;
  virtual void Visit(nodes::AlignofExpr* node) = 0;
  virtual void Visit(nodes::DecltypeExpr* node) = 0;
  virtual void Visit(nodes::ReflectExpr* node) = 0;

  virtual void Visit(nodes::ExprStmt* node) = 0;
  virtual void Visit(nodes::LetStmt* node) = 0;
  virtual void Visit(nodes::AssignStmt* node) = 0;
  virtual void Visit(nodes::WhileStmt* node) = 0;
  virtual void Visit(nodes::ForInStmt* node) = 0;
  virtual void Visit(nodes::BreakStmt* node) = 0;
  virtual void Visit(nodes::ContinueStmt* node) = 0;
  virtual void Visit(nodes::ReturnStmt* node) = 0;
  virtual void Visit(nodes::DeferStmt* node) = 0;
  virtual void Visit(nodes::StaticIfStmt* node) = 0;

  virtual void Visit(nodes::FuncDecl* node) = 0;
  virtual void Visit(nodes::StructDecl* node) = 0;
  virtual void Visit(nodes::EnumDecl* node) = 0;
  virtual void Visit(nodes::ImplDecl* node) = 0;
  virtual void Visit(nodes::TraitDecl* node) = 0;
  virtual void Visit(nodes::TypeAliasDecl* node) = 0;

  virtual void Visit(nodes::PrimitiveType* node) = 0;
  virtual void Visit(nodes::ReferenceType* node) = 0;
  virtual void Visit(nodes::PointerType* node) = 0;
  virtual void Visit(nodes::ArrayType* node) = 0;
  virtual void Visit(nodes::GenericType* node) = 0;
  virtual void Visit(nodes::FunctionType* node) = 0;
  virtual void Visit(nodes::PlaceholderType* node) = 0;

  virtual void Visit(nodes::WildcardPattern* node) = 0;
  virtual void Visit(nodes::BindingPattern* node) = 0;
  virtual void Visit(nodes::LiteralPattern* node) = 0;
  virtual void Visit(nodes::RangePattern* node) = 0;
  virtual void Visit(nodes::TuplePattern* node) = 0;
  virtual void Visit(nodes::StructPattern* node) = 0;
  virtual void Visit(nodes::ReferencePattern* node) = 0;
};

}  // namespace parser::ast::visitor