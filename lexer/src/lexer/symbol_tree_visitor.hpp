#pragma once

#include "visitor.hpp"
#include "scope.hpp"

#include <vector>
#include <string>

#include <memory>

class SymbolTreeVisitor : public Visitor {
 public:
  SymbolTreeVisitor();

  void Visit(LiteralExpression* expr) override;
  void Visit(VariableExpression* expr) override;
  void Visit(BinaryExpression* expr) override;
  void Visit(VarStatement* stmt) override;
  void Visit(AssignStatement* stmt) override;
  void Visit(PrintStatement* stmt) override;
  void Visit(IfStatement* stmt) override;
  void Visit(WhileStatement* stmt) override;
  void Visit(MainFunction* prog) override;

  bool HasErrors() const;
  const std::vector<std::string>& GetErrors() const;
  Scope* GetRootScope() const;

 private:
  void EnterScope();
  void ExitScope();

 private:
  std::unique_ptr<Scope> root_scope_;
  Scope* current_scope_;
  std::vector<std::string> errors_;
};