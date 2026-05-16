#pragma once

#include <common/source_location.hpp>

#include "fwd.hpp"
#include <parser/ast/visitor/visitor.hpp>

#include <vector>

#include <memory>

namespace parser::ast::nodes {

/*
 * base node interface
 */
class Base {
 public:
  explicit Base(common::SourceLocation l)
      : location(std::move(l)) {}

  virtual ~Base() = default;

 public:
  virtual void Accept(visitor::IVisitor* v) = 0;

 public:
  common::SourceLocation location;
  std::vector<std::unique_ptr<Attribute>> attrs;
};

/*
 * all that returns value
 */
class Expression : public Base {
 public:
  using Base::Base;

 public:
  bool is_moved = false;
  bool is_lazy = false;
};

class Statement : public Base {
  using Base::Base;
};

class Declaration : public Statement {
  using Statement::Statement;
};

class TypeNode : public Base { using Base::Base; };

class PatternNode : public Base { using Base::Base; };

}  // namespace parser::ast::nodes