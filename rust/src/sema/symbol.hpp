#pragma once

#include <string>
#include <parser/ast/nodes/fwd.hpp>

#include "type.hpp"

namespace sema {

enum class SymbolKind {
  Variable,
  Function,
  Struct,
  Trait,
  TypeAlias
};

class Symbol {
 public:
  Symbol(std::string name, SymbolKind kind, parser::ast::nodes::Base* decl_node)
      : name_(std::move(name)), kind_(kind), decl_node_(decl_node) {}

 public:
  [[nodiscard]]
  std::string GetName() const {
    return name_;
  }

  [[nodiscard]]
  SymbolKind GetKind() const {
    return kind_;
  }

  [[nodiscard]]
  parser::ast::nodes::Base* GetDeclNode() const {
    return decl_node_;
  }

  [[nodiscard]]
  std::shared_ptr<Type> GetType() const {
    return type_;
  }

  void SetType(std::shared_ptr<Type> type) {
    type_ = std::move(type);
  }

 private:
  std::string name_;
  SymbolKind kind_;
  parser::ast::nodes::Base* decl_node_;
  std::shared_ptr<Type> type_;
};

}  // namespace sema