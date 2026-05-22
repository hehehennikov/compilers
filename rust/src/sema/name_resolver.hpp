#pragma once

#include <parser/ast/visitor/visitor.hpp>
#include "sema/scope.hpp"
#include <stack>

namespace sema {

class NameResolver : public parser::ast::visitor::IVisitor {
 public:
  NameResolver();

  // entry point
  void Resolve(const std::vector<std::unique_ptr<parser::ast::nodes::Base>>& program);

  Scope* GetGlobalScope() { return global_scope_.get(); }

 private:
  // scope management
  void EnterScope();
  void ExitScope();

 private:
  void RegisterGlobalSymbol(parser::ast::nodes::Base* node);
  void RegisterBuiltIn(std::string name, SymbolKind kind);

 public:
  void Visit(parser::ast::nodes::ModuleDecl* node) override;
  void Visit(parser::ast::nodes::ImportDecl* node) override;
  void Visit(parser::ast::nodes::ExportDecl* node) override;
  void Visit(parser::ast::nodes::ExternDecl* node) override;
  void Visit(parser::ast::nodes::Visibility* node) override;

  void Visit(parser::ast::nodes::Attribute* node) override;
  void Visit(parser::ast::nodes::Path* node) override;
  void Visit(parser::ast::nodes::Lifetime* node) override;
  void Visit(parser::ast::nodes::GenericParameter* node) override;
  void Visit(parser::ast::nodes::RequiresClause* node) override;
  void Visit(parser::ast::nodes::WherePredicate* node) override;

  void Visit(parser::ast::nodes::LiteralExpr* node) override;
  void Visit(parser::ast::nodes::IdentExpr* node) override;
  void Visit(parser::ast::nodes::SelfExpr* node) override;
  void Visit(parser::ast::nodes::BorrowExpr* node) override;
  void Visit(parser::ast::nodes::UnitExpr* node) override;

  void Visit(parser::ast::nodes::UnaryExpr* node) override;
  void Visit(parser::ast::nodes::BinaryExpr* node) override;
  void Visit(parser::ast::nodes::SpaceshipExpr* node) override;
  void Visit(parser::ast::nodes::CastExpr* node) override;

  void Visit(parser::ast::nodes::CallExpr* node) override;
  void Visit(parser::ast::nodes::MethodCallExpr* node) override;
  void Visit(parser::ast::nodes::MemberAccessExpr* node) override;
  void Visit(parser::ast::nodes::IndexAccessExpr* node) override;

  void Visit(parser::ast::nodes::BlockExpr* node) override;
  void Visit(parser::ast::nodes::IfExpr* node) override;
  void Visit(parser::ast::nodes::MatchExpr* node) override;
  void Visit(parser::ast::nodes::LoopExpr* node) override;
  void Visit(parser::ast::nodes::TryExpr* node) override;

  void Visit(parser::ast::nodes::NewExpr* node) override;
  void Visit(parser::ast::nodes::LambdaExpr* node) override;

  void Visit(parser::ast::nodes::SizeofExpr* node) override;
  void Visit(parser::ast::nodes::AlignofExpr* node) override;
  void Visit(parser::ast::nodes::DecltypeExpr* node) override;
  void Visit(parser::ast::nodes::ReflectExpr* node) override;

  void Visit(parser::ast::nodes::ExprStmt* node) override;
  void Visit(parser::ast::nodes::LetStmt* node) override;
  void Visit(parser::ast::nodes::AssignStmt* node) override;
  void Visit(parser::ast::nodes::WhileStmt* node) override;
  void Visit(parser::ast::nodes::ForInStmt* node) override;
  void Visit(parser::ast::nodes::BreakStmt* node) override;
  void Visit(parser::ast::nodes::ContinueStmt* node) override;
  void Visit(parser::ast::nodes::ReturnStmt* node) override;
  void Visit(parser::ast::nodes::DeferStmt* node) override;
  void Visit(parser::ast::nodes::StaticIfStmt* node) override;

  void Visit(parser::ast::nodes::FuncDecl* node) override;
  void Visit(parser::ast::nodes::StructDecl* node) override;
  void Visit(parser::ast::nodes::EnumDecl* node) override;
  void Visit(parser::ast::nodes::ImplDecl* node) override;
  void Visit(parser::ast::nodes::TraitDecl* node) override;
  void Visit(parser::ast::nodes::TypeAliasDecl* node) override;

  void Visit(parser::ast::nodes::PrimitiveType* node) override;
  void Visit(parser::ast::nodes::ReferenceType* node) override;
  void Visit(parser::ast::nodes::PointerType* node) override;
  void Visit(parser::ast::nodes::ArrayType* node) override;
  void Visit(parser::ast::nodes::GenericType* node) override;
  void Visit(parser::ast::nodes::FunctionType* node) override;
  void Visit(parser::ast::nodes::PlaceholderType* node) override;

  void Visit(parser::ast::nodes::WildcardPattern* node) override;
  void Visit(parser::ast::nodes::BindingPattern* node) override;
  void Visit(parser::ast::nodes::LiteralPattern* node) override;
  void Visit(parser::ast::nodes::RangePattern* node) override;
  void Visit(parser::ast::nodes::TuplePattern* node) override;
  void Visit(parser::ast::nodes::StructPattern* node) override;
  void Visit(parser::ast::nodes::ReferencePattern* node) override;

 private:
  std::unique_ptr<Scope> global_scope_;
  Scope* current_scope_;
};

}  // namespace sema