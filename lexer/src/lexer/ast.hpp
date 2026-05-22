#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>

struct Context {
  std::map<std::string, int> variables;
};

class Expression {
 public:
  virtual ~Expression() = default;
  virtual int Evaluate(Context& ctx) = 0;
};

class LiteralExpression : public Expression {
 public:
  explicit LiteralExpression(int value)
      : value_(value) {}

  int Evaluate(Context& /* unused */) override {
    return value_;
  }

 private:
  int value_;
};

class VariableExpression : public Expression {
 public:
  explicit VariableExpression(std::string name)
        : name_(std::move(name)) {}

  int Evaluate(Context& ctx) override {
    return ctx.variables[name_];
  }

 private:
  std::string name_;
};

class BinaryExpression : public Expression {
 public:
  BinaryExpression(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
      : left_(std::move(left)), right_(std::move(right)) {}

  int Evaluate(Context& ctx) override {
    return left_->Evaluate(ctx) == right_->Evaluate(ctx);
  }

 private:
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
};

class Statement {
 public:
  virtual ~Statement() = default;
  virtual void Execute(Context& ctx) = 0;
};

class VarStatement : public Statement {
 public:
  explicit VarStatement(std::string name)
      : name_(std::move(name)) {}

  void Execute(Context& ctx) override {
    ctx.variables[name_] = 0;
  }

 private:
  std::string name_;
};

class AssignStatement : public Statement {
 public:
  AssignStatement(std::string name, std::unique_ptr<Expression> expr)
      : name_(std::move(name)), expr_(std::move(expr)) {}

  void Execute(Context& ctx) override {
    ctx.variables[name_] = expr_->Evaluate(ctx);
  }

 private:
  std::string name_;
  std::unique_ptr<Expression> expr_;
};

class PrintStatement : public Statement {
 public:
  explicit PrintStatement(std::unique_ptr<Expression> expr)
      : expr_(std::move(expr)) {}

  void Execute(Context& ctx) override {
    std::cout << expr_->Evaluate(ctx) << '\n';
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

  void Execute(Context& ctx) override {
    if (cond_->Evaluate(ctx)) {
      for (const auto& s : then_branch_) {
        s->Execute(ctx);
      }
    } else {
      for (const auto& s : else_branch_) {
        s->Execute(ctx);
      }
    }
  }

 private:
  std::unique_ptr<Expression> cond_;
  std::vector<std::unique_ptr<Statement>> then_branch_;
  std::vector<std::unique_ptr<Statement>> else_branch_;
};

class Program {
 public:
  virtual ~Program() = default;
  virtual void Run() = 0;
};

class MainFunction : public Program {
 public:
  explicit MainFunction(std::vector<std::unique_ptr<Statement>> stmts)
      : statements_(std::move(stmts)) {}

  void Run() override {
    Context ctx;
    for (const auto& s : statements_) {
      s->Execute(ctx);
    }
  }

 private:
  std::vector<std::unique_ptr<Statement>> statements_;
};