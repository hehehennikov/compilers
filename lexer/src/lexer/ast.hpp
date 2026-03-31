#pragma once

#include <vector>
#include <string>
#include <memory>

#include "visitor.hpp"
#include "scope.hpp"

class Expression {
 public:
  virtual ~Expression() = default;

  virtual void Accept(Visitor* v) = 0;
};

class LiteralExpression : public Expression {
 public:
  explicit LiteralExpression(int value)
    : value_(value) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  int GetValue() const {
    return value_;
  }

 private:
  int value_;
};

class VariableExpression : public Expression {
 public:
  explicit VariableExpression(std::string name)
    : name_(std::move(name)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  const std::string& GetName() const {
    return name_;
  }

  void SetSymbol(Symbol* sym) {
    symbol_ = sym;
  }

  Symbol* GetSymbol() const {
    return symbol_;
  }

 private:
  std::string name_;
  Symbol* symbol_ = nullptr;
};

class BinaryExpression : public Expression {
 public:
  BinaryExpression(std::unique_ptr<Expression> left,
                   std::unique_ptr<Expression> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  Expression* GetLeft() const {
    return left_.get();
  }

  Expression* GetRight() const {
    return right_.get();
  }

 private:
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
};

class Statement {
 public:
  virtual ~Statement() = default;

  virtual void Accept(Visitor* v) = 0;
};

class VarStatement : public Statement {
 public:
  explicit VarStatement(std::string name)
    : name_(std::move(name)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  const std::string& GetName() const {
    return name_;
  }

 private:
  std::string name_;
};

class AssignStatement : public Statement {
 public:
  AssignStatement(std::string name, std::unique_ptr<Expression> expr)
      : name_(std::move(name)), expr_(std::move(expr)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  const std::string& GetName() const {
    return name_;
  }

  Expression* GetExpression() const {
    return expr_.get();
  }

  void SetSymbol(Symbol* sym) {
    symbol_ = sym;
  }

  Symbol* GetSymbol() const {
    return symbol_;
  }

 private:
  std::string name_;
  std::unique_ptr<Expression> expr_;
  Symbol* symbol_ = nullptr;
};

class PrintStatement : public Statement {
 public:
  explicit PrintStatement(std::unique_ptr<Expression> expr)
    : expr_(std::move(expr)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  Expression* GetExpression() const {
    return expr_.get();
  }

 private:
  std::unique_ptr<Expression> expr_;
};

class IfStatement : public Statement {
 public:
  IfStatement(std::unique_ptr<Expression> cond,
              std::vector<std::unique_ptr<Statement>> then_branch,
              std::vector<std::unique_ptr<Statement>> else_branch)
      : cond_(std::move(cond)),
        then_branch_(std::move(then_branch)),
        else_branch_(std::move(else_branch)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  Expression* GetCondition() const {
    return cond_.get();
  }

  const std::vector<std::unique_ptr<Statement>>& GetThenBranch() const {
    return then_branch_;
  }

  const std::vector<std::unique_ptr<Statement>>& GetElseBranch() const {
    return else_branch_;
  }

 private:
  std::unique_ptr<Expression> cond_;
  std::vector<std::unique_ptr<Statement>> then_branch_;
  std::vector<std::unique_ptr<Statement>> else_branch_;
};

class WhileStatement : public Statement {
 public:
  WhileStatement(std::unique_ptr<Expression> cond,
                 std::vector<std::unique_ptr<Statement>> body)
      : cond_(std::move(cond)), body_(std::move(body)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  Expression* GetCondition() const {
    return cond_.get();
  }

  const std::vector<std::unique_ptr<Statement>>& GetBody() const {
    return body_;
  }

 private:
  std::unique_ptr<Expression> cond_;
  std::vector<std::unique_ptr<Statement>> body_;
};

class Program {
 public:
  virtual ~Program() = default;

  virtual void Accept(Visitor* v) = 0;
};

class MainFunction : public Program {
 public:
  explicit MainFunction(std::vector<std::unique_ptr<Statement>> stmts)
      : statements_(std::move(stmts)) {}

  void Accept(Visitor* v) override {
    v->Visit(this);
  }

  const std::vector<std::unique_ptr<Statement>>& GetStatements() const {
    return statements_;
  }

 private:
  std::vector<std::unique_ptr<Statement>> statements_;
};