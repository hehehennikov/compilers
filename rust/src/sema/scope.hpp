#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "symbol.hpp"

namespace sema {

class Scope {
 public:
  explicit Scope(Scope* parent = nullptr) : parent_(parent) {}

  /*
   * defines a new symbol in the current scope
   * if the name exists, it shadows the previous definition.
   */
  void Define(std::unique_ptr<Symbol> symbol) {
    std::string name = symbol->GetName();
    symbols_[name] = std::move(symbol);
  }

  /*
   * look up a symbol by name, climbing up the scope hierarchy
   */
  Symbol* Lookup(const std::string& name) const {
    auto it = symbols_.find(name);

    if (it != symbols_.end()) {
      return it->second.get();
    }

    if (parent_ != nullptr) {
      return parent_->Lookup(name);
    }

    return nullptr;
  }

  Scope* CreateChild() {
    children_.push_back(std::make_unique<Scope>(this));
    return children_.back().get();
  }

  Scope* GetParent() const {
    return parent_;
  }

  const std::unordered_map<std::string, std::unique_ptr<Symbol>>& GetLocalSymbols() const {
    return symbols_;
  }

 private:
  Scope* parent_;
  std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols_;
  // scope owns his children
  std::vector<std::unique_ptr<Scope>> children_;
};

}  // namespace sema