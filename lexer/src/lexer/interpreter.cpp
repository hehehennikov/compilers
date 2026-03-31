#include "interpreter.hpp"
#include "ast.hpp"

#include <iostream>

void Interpreter::Visit(LiteralExpression* expr) {
  last_result_ = expr->GetValue();
}

void Interpreter::Visit(VariableExpression* expr) {
  Symbol* sym = expr->GetSymbol();
  last_result_ = memory_[sym]; // memory_ это map<Symbol*, int>
}

void Interpreter::Visit(BinaryExpression* expr) {
  expr->GetLeft()->Accept(this);
  auto left = last_result_;
  expr->GetRight()->Accept(this);
  auto right = last_result_;
  last_result_ = (left == right) ? 1 : 0;
}

void Interpreter::Visit(VarStatement* stmt) {}

void Interpreter::Visit(AssignStatement* stmt) {
  stmt->GetExpression()->Accept(this);
  Symbol* sym = stmt->GetSymbol();
  memory_[sym] = last_result_;
}

void Interpreter::Visit(PrintStatement* stmt) {
  stmt->GetExpression()->Accept(this);
  std::cout << last_result_ << std::endl;
}

void Interpreter::Visit(IfStatement* stmt) {
  stmt->GetCondition()->Accept(this);
  if (last_result_ != 0) {
    for (const auto& s : stmt->GetThenBranch()) {
      s->Accept(this);
    }
  } else {
    for (const auto& s : stmt->GetElseBranch()) {
      s->Accept(this);
    }
  }
}

void Interpreter::Visit(WhileStatement* stmt) {
  while (true) {
    stmt->GetCondition()->Accept(this);
    if (last_result_ == 0) {
      break;
    }
    for (const auto& s : stmt->GetBody()) {
      s->Accept(this);
    }
  }
}

void Interpreter::Visit(MainFunction* prog) {
  for (const auto& stmt : prog->GetStatements()) {
    stmt->Accept(this);
  }
}