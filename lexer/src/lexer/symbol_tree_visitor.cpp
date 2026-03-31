#include "symbol_tree_visitor.hpp"
#include "ast.hpp"

#include <format>

SymbolTreeVisitor::SymbolTreeVisitor() {
  root_scope_ = std::make_unique<Scope>();
  current_scope_ = root_scope_.get();
}

void SymbolTreeVisitor::EnterScope() {
  current_scope_ = current_scope_->AddChild();
}

void SymbolTreeVisitor::ExitScope() {
  if (current_scope_->GetParent()) {
    current_scope_ = current_scope_->GetParent();
  }
}

bool SymbolTreeVisitor::HasErrors() const {
  return not errors_.empty();
}

const std::vector<std::string>& SymbolTreeVisitor::GetErrors() const {
  return errors_;
}

Scope* SymbolTreeVisitor::GetRootScope() const {
  return root_scope_.get();
}

void SymbolTreeVisitor::Visit(LiteralExpression*) {}

// В symbol_tree_visitor.cpp
void SymbolTreeVisitor::Visit(VariableExpression* expr) {
  Symbol* sym = current_scope_->Lookup(expr->GetName());
  if (not sym) {
    errors_.push_back("Undeclared variable: " + expr->GetName());
  } else {
    if (not sym->is_initialized) {
      errors_.push_back("Error: Variable '" + expr->GetName() + "' is used before being initialized.");
    }
    expr->SetSymbol(sym);
  }
}

void SymbolTreeVisitor::Visit(BinaryExpression* expr) {
  expr->GetLeft()->Accept(this);
  expr->GetRight()->Accept(this);
}

void SymbolTreeVisitor::Visit(VarStatement* stmt) {
  Symbol* sym = current_scope_->Declare(stmt->GetName(), SymbolType::Variable, "int");
  if (not sym) {
    errors_.push_back(std::format("Error: Variable '{}' is already declared in this scope.", stmt->GetName()));
  }
}

void SymbolTreeVisitor::Visit(AssignStatement* stmt) {
  Symbol* sym = current_scope_->Lookup(stmt->GetName());
  if (not sym) {
    errors_.push_back("Error: Cannot assign to undeclared '" + stmt->GetName() + "'.");
  } else {
    sym->is_initialized = true;
    stmt->SetSymbol(sym);
  }
  stmt->GetExpression()->Accept(this);
}

void SymbolTreeVisitor::Visit(PrintStatement* stmt) {
  stmt->GetExpression()->Accept(this);
}

void SymbolTreeVisitor::Visit(IfStatement* stmt) {
  stmt->GetCondition()->Accept(this);

  EnterScope();
  for (const auto& s : stmt->GetThenBranch()) {
    s->Accept(this);
  }
  ExitScope();

  if (not stmt->GetElseBranch().empty()) {
    EnterScope();
    for (const auto& s : stmt->GetElseBranch()) {
      s->Accept(this);
    }
    ExitScope();
  }
}

void SymbolTreeVisitor::Visit(WhileStatement* stmt) {
  stmt->GetCondition()->Accept(this);
  EnterScope();
  for (const auto& s : stmt->GetBody()) {
    s->Accept(this);
  }
  ExitScope();
}

void SymbolTreeVisitor::Visit(MainFunction* node) {
  for (auto& stmt : node->GetStatements()) {
    stmt->Accept(this);
  }
}