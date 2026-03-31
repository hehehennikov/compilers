#pragma once

class LiteralExpression;
class VariableExpression;
class BinaryExpression;
class VarStatement;
class AssignStatement;
class PrintStatement;
class IfStatement;
class WhileStatement;
class MainFunction;

class Visitor {
public:
  virtual ~Visitor() = default;
  virtual void Visit(LiteralExpression* expr) = 0;
  virtual void Visit(VariableExpression* expr) = 0;
  virtual void Visit(BinaryExpression* expr) = 0;
  virtual void Visit(VarStatement* stmt) = 0;
  virtual void Visit(AssignStatement* stmt) = 0;
  virtual void Visit(PrintStatement* stmt) = 0;
  virtual void Visit(IfStatement* stmt) = 0;
  virtual void Visit(WhileStatement* stmt) = 0;
  virtual void Visit(MainFunction* prog) = 0;
};