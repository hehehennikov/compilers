#pragma once

#include "base.hpp"
#include "misc.hpp"

#include <lexer/token.hpp>

namespace parser::ast::nodes {

class PrimitiveType : public Type {
 public:
  PrimitiveType(lexer::TokenType kind, common::SourceLocation l)
      : Type(l), kind(kind) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  lexer::TokenType kind;
};

class ReferenceType : public Type {
 public:
  ReferenceType(bool is_mutable, 
                std::unique_ptr<Lifetime> lifetime, 
                std::unique_ptr<Type> base,
                common::SourceLocation l)
      : Type(l),
        lifetime(std::move(lifetime)),
        base(std::move(base)),
        is_mutable(is_mutable) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Lifetime> lifetime;
  std::unique_ptr<Type> base;
  bool is_mutable;
};

class PointerType : public Type {
 public:
  enum class Kind {
    RawConst,
    RawMut,
    Box,
    Unique
  };

 public:
  PointerType(Kind kind, std::unique_ptr<Type> base, common::SourceLocation l)
      : Type(l), kind(kind), base(std::move(base)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  Kind kind;
  std::unique_ptr<Type> base;
};

class ArrayType : public Type {
 public:
  ArrayType(std::unique_ptr<Type> base,
            std::unique_ptr<Expression> size_expr, 
            common::SourceLocation l)
      : Type(l),
        base(std::move(base)), 
        size_expr(std::move(size_expr)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Type> base;
  std::unique_ptr<Expression> size_expr;
};

/*
 * ex.: Vec<i32>, Result<T, E>.
 */
class GenericType : public Type {
 public:
  GenericType(std::unique_ptr<Path> path, 
              std::vector<std::unique_ptr<Type>> arguments,
              common::SourceLocation l)
      : Type(l),
        path(std::move(path)), 
        arguments(std::move(arguments)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Path> path;
  std::vector<std::unique_ptr<Type>> arguments;
};

class FunctionType : public Type {
 public:
  FunctionType(std::vector<std::unique_ptr<Type>> params,
               std::unique_ptr<Type> return_type,
               common::SourceLocation l)
      : Type(l),
        params(std::move(params)), 
        return_type(std::move(return_type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::vector<std::unique_ptr<Type>> params;
  std::unique_ptr<Type> return_type; // may be nullptr for unit
};

class PlaceholderType : public Type {
 public:
  explicit PlaceholderType(common::SourceLocation l) : Type(l) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }
};

}  // namespace parser::ast::nodes