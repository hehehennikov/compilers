#pragma once

#include "base.hpp"

namespace parser::ast::nodes {

class LetStmt : public Statement {
 public:
  LetStmt(std::unique_ptr<PatternNode> pat,
          bool mut,
          bool lazy,
          std::unique_ptr<TypeNode> type,
          std::unique_ptr<Expression> init,
          common::SourceLocation l)
      : Statement(std::move(l)),
        pattern(std::move(pat)),
        is_mutable(mut),
        is_lazy(lazy),
        type_ann(std::move(type)),
        initializer(std::move(init)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<PatternNode> pattern;
  bool is_mutable;
  bool is_lazy;
  std::unique_ptr<TypeNode> type_ann;
  std::unique_ptr<Expression> initializer;
};

class AssignStmt : public Statement {
 public:
  AssignStmt(std::unique_ptr<Expression> lhs,
             std::unique_ptr<Expression> rhs,
             common::SourceLocation l)
      : Statement(std::move(l)),
        lhs(std::move(lhs)),
        rhs(std::move(rhs)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> lhs;
  std::unique_ptr<Expression> rhs;
};

class ReturnStmt : public Statement {
 public:
  ReturnStmt(std::unique_ptr<Expression> val, common::SourceLocation l)
      : Statement(std::move(l)), value(std::move(val)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> value;
};

class WhileStmt : public Statement {
 public:
  WhileStmt(std::unique_ptr<Expression> cond,
            std::unique_ptr<BlockExpr> body,
            common::SourceLocation l)
      : Statement(std::move(l)),
        condition(std::move(cond)),
        body(std::move(body)) {}

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
  ForInStmt(std::unique_ptr<PatternNode> pat, std::unique_ptr<Expression> iter,
          std::unique_ptr<BlockExpr> body, common::SourceLocation l)
      : Statement(std::move(l)),
        pattern(std::move(pat)),
        iterable(std::move(iter)),
        body(std::move(body)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<PatternNode> pattern;
  std::unique_ptr<Expression> iterable;
  std::unique_ptr<BlockExpr> body;
};

class LoopStmt : public Statement {
 public:
  LoopStmt(std::unique_ptr<BlockExpr> body, common::SourceLocation l)
      : Statement(std::move(l)), body(std::move(body)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<BlockExpr> body;
};

class BreakStmt : public Statement {
 public:
  explicit BreakStmt(common::SourceLocation l)
      : Statement(std::move(l)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }
};

class ContinueStmt : public Statement {
 public:
  explicit ContinueStmt(common::SourceLocation l)
      : Statement(std::move(l)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }
};

class StaticIfStmt : public Statement {
 public:
  StaticIfStmt(std::unique_ptr<Expression> cond,
               std::unique_ptr<BlockExpr> then_block,
               std::unique_ptr<BlockExpr> else_block,
               common::SourceLocation l)
      : Statement(std::move(l)),
        condition(std::move(cond)),
        then_branch(std::move(then_block)),
        else_branch(std::move(else_block)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<BlockExpr> then_branch;
  std::unique_ptr<BlockExpr> else_branch; // Может быть nullptr
};

class ExprStmt : public Statement {
 public:
  ExprStmt(std::unique_ptr<Expression> e, common::SourceLocation l)
      : Statement(std::move(l)), expr(std::move(e)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> expr;
};

}  // namespace parser::ast::nodes