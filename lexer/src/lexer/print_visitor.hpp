#pragma once

#include "visitor.hpp"
#include <fstream>
#include <string>

class PrintVisitor : public Visitor {
 public:
  explicit PrintVisitor(const std::string& filename);
  ~PrintVisitor();

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
  void PrintIndent();

  std::ofstream out_;
  int indent_ = 0;
};