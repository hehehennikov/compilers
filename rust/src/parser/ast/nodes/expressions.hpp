#pragma once

#include "base.hpp"

#include "misc.hpp"

#include <lexer/token.hpp>

#include <sema/scope.hpp>

namespace parser::ast::nodes {

class LiteralExpr : public Expression {
 public:
  LiteralExpr(lexer::TokenData value, common::SourceLocation l)
      : Expression(l), value(std::move(value)) {}

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
      : Expression(l), name(std::move(name)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  sema::Symbol* symbol = nullptr;
};

class SelfExpr : public Expression {
 public:
  explicit SelfExpr(common::SourceLocation l)
      : Expression(l) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  sema::Symbol* symbol = nullptr;
};

class BorrowExpr : public Expression {
 public:
  BorrowExpr(bool is_mut, std::unique_ptr<Expression> operand, common::SourceLocation l)
      : Expression(l), operand(std::move(operand)), is_mutable(is_mut) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> operand;
  bool is_mutable;
};

class UnitExpr : public Expression {
 public:
  explicit UnitExpr(common::SourceLocation l)
      : Expression(l) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }
};

class UnaryExpr : public Expression {
 public:
  UnaryExpr(lexer::TokenType op,
            std::unique_ptr<Expression> operand,
            common::SourceLocation l)
      : Expression(l), op(op), operand(std::move(operand)) {}

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
      : Expression(l), left(std::move(left)), op(op), right(std::move(right)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> left;
  lexer::TokenType op;
  std::unique_ptr<Expression> right;
};

class SpaceshipExpr : public Expression {
 public:
  SpaceshipExpr(std::unique_ptr<Expression> left,
                std::unique_ptr<Expression> right,
                common::SourceLocation l)
      : Expression(l), left(std::move(left)), right(std::move(right)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> left;
  std::unique_ptr<Expression> right;
};

class CastExpr : public Expression {
 public:
  CastExpr(std::unique_ptr<Expression> expr,
           std::unique_ptr<Type> target_type,
           common::SourceLocation l)
      : Expression(l),
        expr(std::move(expr)),
        target_type(std::move(target_type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> expr;
  std::unique_ptr<Type> target_type;
};

class CallExpr : public Expression {
 public:
  CallExpr(std::unique_ptr<Expression> callee,
           std::vector<std::unique_ptr<Expression>> arguments,
           common::SourceLocation l)
      : Expression(l),
        callee(std::move(callee)),
        arguments(std::move(arguments)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> callee;
  std::vector<std::unique_ptr<Expression>> arguments;
};

class MethodCallExpr : public Expression {
 public:
  MethodCallExpr(std::unique_ptr<Expression> object,
                 std::string method_name,
                 std::vector<std::unique_ptr<Expression>> arguments,
                 common::SourceLocation l)
      : Expression(l), object(std::move(object)),
        method_name(std::move(method_name)),
        arguments(std::move(arguments)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> object;
  std::string method_name;
  std::vector<std::unique_ptr<Expression>> arguments;
  sema::Symbol* resolved_method = nullptr;
};

class MemberAccessExpr : public Expression {
 public:
  MemberAccessExpr(std::unique_ptr<Expression> object,
                   std::string member_name,
                   common::SourceLocation l)
      : Expression(l),
        object(std::move(object)),
        member_name(std::move(member_name)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> object;
  std::string member_name;
};

class IndexAccessExpr : public Expression {
 public:
  IndexAccessExpr(std::unique_ptr<Expression> object,
                  std::unique_ptr<Expression> index,
                  common::SourceLocation l)
      : Expression(l), object(std::move(object)), index(std::move(index)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> object;
  std::unique_ptr<Expression> index;
};

class BlockExpr : public Expression {
 public:
  BlockExpr(std::vector<std::unique_ptr<Statement>> statements,
            std::unique_ptr<Expression> final_expression,
            common::SourceLocation l)
      : Expression(l), statements(std::move(statements)),
        final_expression(std::move(final_expression)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::vector<std::unique_ptr<Statement>> statements;
  std::unique_ptr<Expression> final_expression; // may be nullptr
  sema::Scope* scope = nullptr;
};

class IfExpr : public Expression {
 public:
  IfExpr(std::unique_ptr<Expression> condition, std::unique_ptr<BlockExpr> then_branch,
         std::unique_ptr<BlockExpr> else_branch, common::SourceLocation l)
      : Expression(l), condition(std::move(condition)),
        then_branch(std::move(then_branch)), else_branch(std::move(else_branch)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> condition;
  std::unique_ptr<BlockExpr> then_branch;
  std::unique_ptr<BlockExpr> else_branch; // may be nullptr
};

class MatchExpr : public Expression {
 public:
  MatchExpr(std::unique_ptr<Expression> value, std::vector<MatchArm> arms, common::SourceLocation l)
      : Expression(l), value(std::move(value)), arms(std::move(arms)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> value;
  std::vector<MatchArm> arms;
};

class LoopExpr : public Expression {
 public:
  LoopExpr(std::unique_ptr<BlockExpr> body, common::SourceLocation l)
      : Expression(l), body(std::move(body)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<BlockExpr> body;
};

class TryExpr : public Expression {
 public:
  TryExpr(std::unique_ptr<Expression> expr, common::SourceLocation l)
      : Expression(l), expr(std::move(expr)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> expr; // for `?` operator
};

class NewExpr : public Expression {
 public:
  NewExpr(std::unique_ptr<Type> type,
          std::vector<std::unique_ptr<Expression>> args,
          common::SourceLocation l)
      : Expression(l), target_type(std::move(type)), arguments(std::move(args)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Type> target_type;
  std::vector<std::unique_ptr<Expression>> arguments;
};

class LambdaExpr : public Expression {
 public:
  LambdaExpr(bool is_move, std::vector<GenericParameter> params,
             std::unique_ptr<BlockExpr> body, common::SourceLocation l)
      : Expression(l),
        params(std::move(params)),
        body(std::move(body)),
        is_move(is_move) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::vector<GenericParameter> params;
  std::unique_ptr<BlockExpr> body;
  bool is_move;
};

class SizeofExpr : public Expression {
 public:
  SizeofExpr(std::unique_ptr<Type> type, common::SourceLocation l)
      : Expression(l), target_type(std::move(type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Type> target_type;
};

class AlignofExpr : public Expression {
 public:
  AlignofExpr(std::unique_ptr<Type> type, common::SourceLocation l)
      : Expression(l), target_type(std::move(type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Type> target_type;
};

class DecltypeExpr : public Expression {
 public:
  DecltypeExpr(std::unique_ptr<Expression> expr, common::SourceLocation l)
      : Expression(l), expr(std::move(expr)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> expr;
};

class ReflectExpr : public Expression {
 public:
  ReflectExpr(std::unique_ptr<Type> type, common::SourceLocation l)
      : Expression(l), target_type(std::move(type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Type> target_type;
};

}  // namespace parser::ast::nodes