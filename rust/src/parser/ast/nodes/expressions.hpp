#pragma once

#include "base.hpp"

#include <lexer/token.hpp>

namespace parser::ast::nodes {

class LiteralExpr : public Expression {
 public:
  LiteralExpr(lexer::TokenData val, common::SourceLocation l)
      : Expression(std::move(l)), value(std::move(val)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  lexer::TokenData value;
};

class IdentExpr : public Expression {
 public:
  IdentExpr(std::string name, common::SourceLocation l)
      : Expression(std::move(l)), name(std::move(name)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
};

class UnaryExpr : public Expression {
 public:
  UnaryExpr(lexer::TokenType op,
            std::unique_ptr<Expression> operand,
            common::SourceLocation l)
      : Expression(std::move(l)), op(op), operand(std::move(operand)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  lexer::TokenType op;
  std::unique_ptr<Expression> operand;
};

class BinaryExpr : public Expression {
 public:
  BinaryExpr(std::unique_ptr<Expression> left, lexer::TokenType op,
             std::unique_ptr<Expression> right, common::SourceLocation l)
      : Expression(std::move(l)), left(std::move(left)), right(std::move(right)), op(op) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> left;
  std::unique_ptr<Expression> right;
  lexer::TokenType op;
};

class CastExpr : public Expression {
 public:
  CastExpr(std::unique_ptr<Expression> expr,
           std::unique_ptr<TypeNode> target_type,
           common::SourceLocation l)
      : Expression(std::move(l)),
        expr(std::move(expr)),
        target_type(std::move(target_type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> expr;
  std::unique_ptr<TypeNode> target_type;
};

class MemberAccessExpr : public Expression {
 public:
  MemberAccessExpr(std::unique_ptr<Expression> object, std::string member, common::SourceLocation l)
      : Expression(std::move(l)), object(std::move(object)), member(std::move(member)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> object;
  std::string member;
};

class CallExpr : public Expression {
 public:
  CallExpr(std::unique_ptr<Expression> callee, std::vector<std::unique_ptr<Expression>> args, common::SourceLocation l)
      : Expression(std::move(l)), callee(std::move(callee)), arguments(std::move(args)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> callee;
  std::vector<std::unique_ptr<Expression>> arguments;
};

class BlockExpr : public Expression {
 public:
  BlockExpr(std::vector<std::unique_ptr<Statement>> stmts,
            std::unique_ptr<Expression> final_expr, common::SourceLocation l)
      : Expression(std::move(l)), statements(std::move(stmts)), final_expression(std::move(final_expr)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::vector<std::unique_ptr<Statement>> statements;
  std::unique_ptr<Expression> final_expression; // может быть nullptr
};

class IfExpr : public Expression {
 public:
  IfExpr(std::unique_ptr<Expression> cond, std::unique_ptr<BlockExpr> then_b,
         std::unique_ptr<BlockExpr> else_b, common::SourceLocation l)
      : Expression(std::move(l)),
        condition(std::move(cond)),
        then_block(std::move(then_b)),
        else_block(std::move(else_b)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<BlockExpr> then_block;
  std::unique_ptr<BlockExpr> else_block;
};

struct MatchArm {
  std::unique_ptr<PatternNode> pattern;
  std::unique_ptr<Expression> body;
};

class MatchExpr : public Expression {
 public:
  MatchExpr(std::unique_ptr<Expression> val, std::vector<MatchArm> arms, common::SourceLocation l)
      : Expression(std::move(l)), value(std::move(val)), arms(std::move(arms)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> value;
  std::vector<MatchArm> arms;
};

class NewExpr : public Expression {
 public:
  NewExpr(std::unique_ptr<TypeNode> type,
          std::vector<std::unique_ptr<Expression>> args,
          common::SourceLocation l)
      : Expression(std::move(l)),
        target_type(std::move(type)),
        arguments(std::move(args)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<TypeNode> target_type;
  std::vector<std::unique_ptr<Expression>> arguments;
};

class TryExpr : public Expression {
 public:
  TryExpr(std::unique_ptr<Expression> e, common::SourceLocation l)
      : Expression(std::move(l)), expr(std::move(e)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> expr;
};


}  // namespace parser::ast::nodes