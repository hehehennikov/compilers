#pragma once

#include "base.hpp"
#include "misc.hpp"

#include <lexer/token.hpp>

namespace parser::ast::nodes {

class PrimitiveType : public TypeNode {
 public:
  PrimitiveType(const lexer::TokenType& kind, common::SourceLocation l)
      : TypeNode(std::move(l)), kind(kind) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  lexer::TokenType kind;
};

class ReferenceType : public TypeNode {
 public:
  ReferenceType(bool is_mut,
                    std::unique_ptr<Lifetime> lt,
                    std::unique_ptr<TypeNode> base,
                    common::SourceLocation l)
      : TypeNode(std::move(l)),
        is_mutable(is_mut),
        lifetime(std::move(lt)),
        base(std::move(base)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  bool is_mutable;
  std::unique_ptr<Lifetime> lifetime;
  std::unique_ptr<TypeNode> base;
};

class PointerType : public TypeNode {
 public:
  enum class Kind {
    Raw,
    Box,
    Unique
  };

 public:
  PointerType(const Kind& k,
                  std::unique_ptr<TypeNode> base,
                  common::SourceLocation l)
      : TypeNode(std::move(l)), kind(k), base(std::move(base)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  Kind kind;
  std::unique_ptr<TypeNode> base;
};

class ArrayType : public TypeNode {
 public:
  ArrayType(std::unique_ptr<TypeNode> base,
                std::unique_ptr<Expression> size,
                common::SourceLocation l)
      : TypeNode(std::move(l)), base(std::move(base)), size_expr(std::move(size)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<TypeNode> base;
  std::unique_ptr<Expression> size_expr;
};

class GenericType : public TypeNode {
 public:
  GenericType(std::string name,
                  std::vector<std::unique_ptr<TypeNode>> args,
                  common::SourceLocation l)
      : TypeNode(std::move(l)), base_name(std::move(name)), arguments(std::move(args)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string base_name;
  std::vector<std::unique_ptr<TypeNode>> arguments;
};

class FunctionType : public TypeNode {
 public:
  FunctionType(std::vector<std::unique_ptr<TypeNode>> params,
               std::unique_ptr<TypeNode> ret, common::SourceLocation l)
      : TypeNode(std::move(l)),
        param_types(std::move(params)),
        return_type(std::move(ret)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::vector<std::unique_ptr<TypeNode>> param_types;
  std::unique_ptr<TypeNode> return_type;
};

}  // namespace parser::ast::nodes