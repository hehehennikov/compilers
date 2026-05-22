#pragma once

#include "base.hpp"

#include <lexer/lexer.hpp>

namespace parser::ast::nodes {

/*
 * expression wrapper, makes possible to use it like an instruction
 */
class ExprStmt : public Statement {
 public:
  ExprStmt(std::unique_ptr<Expression> expr, common::SourceLocation l)
      : Statement(l), expr(std::move(expr)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> expr;
};

class LetStmt : public Statement {
 public:
  LetStmt(std::unique_ptr<Pattern> pattern,
          bool is_mutable,
          std::unique_ptr<Type> type_ann,
          std::unique_ptr<Expression> initializer,
          common::SourceLocation l)
      : Statement(l),
        pattern(std::move(pattern)),
        type_ann(std::move(type_ann)),
        initializer(std::move(initializer)),
        is_mutable(is_mutable) {}

 public:
  void Accept(visitor::IVisitor* v) override { v->Visit(this); }

  std::unique_ptr<Pattern> pattern; // binding pattern (ex.: `let (x, y) = get_pair();`)
  std::unique_ptr<Type> type_ann;    // may be nullptr (for _)
  std::unique_ptr<Expression> initializer; // may be nullptr (ex.: `let x: i32;`)
  bool is_mutable;
};

class AssignStmt : public Statement {
 public:
  AssignStmt(std::unique_ptr<Expression> lhs,
             lexer::TokenType op,
             std::unique_ptr<Expression> rhs,
             common::SourceLocation l)
      : Statement(l), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> lhs; // must be lvalue
  lexer::TokenType op; // (=, +=, -=, &=, etc)
  std::unique_ptr<Expression> rhs;
};

class WhileStmt : public Statement {
 public:
  WhileStmt(std::unique_ptr<Expression> condition,
            std::unique_ptr<BlockExpr> body,
            common::SourceLocation l)
      : Statement(l), condition(std::move(condition)), body(std::move(body)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<BlockExpr> body;
};

class ForInStmt : public Statement {
 public:
  ForInStmt(std::unique_ptr<Pattern> pattern,
            std::unique_ptr<Expression> iterable,
            std::unique_ptr<BlockExpr> body,
            common::SourceLocation l)
      : Statement(l),
        pattern(std::move(pattern)),
        iterable(std::move(iterable)),
        body(std::move(body)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Pattern> pattern;
  std::unique_ptr<Expression> iterable;
  std::unique_ptr<BlockExpr> body;
};

class BreakStmt : public Statement {
 public:
  BreakStmt(std::unique_ptr<Expression> value, common::SourceLocation l)
      : Statement(l), value(std::move(value)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> value; // break may return value
};

class ContinueStmt : public Statement {
 public:
  explicit ContinueStmt(common::SourceLocation l)
      : Statement(l) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }
};

class ReturnStmt : public Statement {
 public:
  ReturnStmt(std::unique_ptr<Expression> value, common::SourceLocation l)
      : Statement(l), value(std::move(value)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> value; // may be nullptr for unit return
};

class DeferStmt : public Statement {
 public:
  DeferStmt(std::unique_ptr<BlockExpr> body, common::SourceLocation l)
      : Statement(l), body(std::move(body)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<BlockExpr> body;
};

class StaticIfStmt : public Statement {
 public:
  StaticIfStmt(std::unique_ptr<Expression> condition,
               std::unique_ptr<BlockExpr> then_branch,
               std::unique_ptr<BlockExpr> else_branch,
               common::SourceLocation l)
      : Statement(l),
        condition(std::move(condition)),
        then_branch(std::move(then_branch)),
        else_branch(std::move(else_branch)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<BlockExpr> then_branch;
  std::unique_ptr<BlockExpr> else_branch; // may be nullptr
};

}  // namespace parser::ast::nodes