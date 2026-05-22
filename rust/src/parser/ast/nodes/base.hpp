#pragma once

#include <common/source_location.hpp>

#include "fwd.hpp"

#include <sema/type.hpp>

#include <parser/ast/visitor/visitor.hpp>

#include <vector>

#include <memory>

namespace parser::ast::nodes {

/*
 * base node interface
 */
class Base {
 public:
  virtual ~Base() = default;

 public:
  explicit Base(common::SourceLocation l);

  Base(Base&&) noexcept;
  Base& operator=(Base&&) noexcept;

  Base(const Base&) = delete;
  Base& operator=(const Base&) = delete;

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
  using Base::Base;

 public:
  void SetResolvedType(std::shared_ptr<sema::Type> type) {
    resolved_type_ = std::move(type);
  }

  [[nodiscard]]
  auto GetResolvedType() const -> std::shared_ptr<sema::Type> {
    return resolved_type_;
  }

  std::shared_ptr<sema::Type> resolved_type_ = nullptr;

 public:
  bool is_moved = false;
  std::shared_ptr<sema::Type> resolved_type = nullptr;
};

/*
 * instructions
 */
class Statement : public Base {
  using Base::Base;
};

class Declaration : public Statement {
  using Statement::Statement;
};

class Type : public Base {
  using Base::Base;
};

class Pattern : public Base {
  using Base::Base;

 public:
  void SetResolvedType(std::shared_ptr<sema::Type> type) {
    resolved_type_ = std::move(type);
  }

  [[nodiscard]]
  auto GetResolvedType() const {
    return resolved_type_;
  }

 public:
  std::shared_ptr<sema::Type> resolved_type_ = nullptr;
};

class ModuleItem : public Base {
  using Base::Base;
};

}  // namespace parser::ast::nodes