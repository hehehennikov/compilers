#pragma once

#include "visitor.hpp"
#include <map>
#include <string>
#include "symbol_tree_visitor.hpp"

class Interpreter : public Visitor {
 public:
  void Visit(LiteralExpression* expr) override;
  void Visit(VariableExpression* expr) override;
  void Visit(BinaryExpression* expr) override;
  void Visit(VarStatement* stmt) override;
  void Visit(AssignStatement* stmt) override;
  void Visit(PrintStatement* stmt) override;
  void Visit(IfStatement* stmt) override;
  void Visit(WhileStatement* stmt) override;
  void Visit(MainFunction* prog) override;

 private:
  std::unordered_map<Symbol*, int> memory_;
  int last_result_ = 0;
};