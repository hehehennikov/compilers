#pragma once

#include "base.hpp"

#include "fwd.hpp"

namespace parser::ast::nodes {

class Attribute : public Base {
 public:
  Attribute(std::string attr,
            std::vector<std::unique_ptr<Expression>> args,
            common::SourceLocation l)
      : Base(std::move(l)), name(std::move(attr)), args(std::move(args)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<Expression>> args;
};

class Lifetime : public Base {
 public:
  Lifetime(std::string name, common::SourceLocation l)
      : Base(std::move(l)), name(std::move(name)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
};

class GenericParameter : public Base  {
 public:
  GenericParameter(std::string name,
                       std::vector<std::unique_ptr<TypeNode>> bounds,
                       common::SourceLocation l)
                         : Base(std::move(l)),
                           name(std::move(name)),
                           bounds(std::move(bounds)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit();
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<TypeNode>> bounds;
};

class WherePredicate : public Base {
 public:
  WherePredicate(std::unique_ptr<TypeNode> type,
                     std::vector<std::unique_ptr<TypeNode>> bounds,
                     common::SourceLocation l)
      : Base(std::move(l)),
        target_type(std::move(type)),
        bounds(std::move(bounds)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<TypeNode> target_type;
  std::vector<std::unique_ptr<TypeNode>> bounds;
};

}  // namespace parser::ast::nodes