#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <memory>

enum class SymbolType {
  None,
  Variable,
  Method,
  Class
};

struct Symbol {
  std::string name;
  SymbolType type = SymbolType::None;
  std::string data_type;
  bool is_initialized = false;
};

class Scope {
 public:
  explicit Scope(Scope* parent = nullptr)
    : parent_(parent) {}

  Symbol* Declare(const std::string& name, SymbolType type, const std::string& data_type) {
    if (symbols_.contains(name)) {
      return nullptr;
    }

    symbols_[name] = Symbol{name, type, data_type, false};
    return &symbols_[name];
  }

  Symbol* Lookup(const std::string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
      return &(it->second);
    }

    if (parent_) {
      return parent_->Lookup(name);
    }

    return nullptr;
  }


  Scope* GetParent() const {
    return parent_;
  }

  Scope* AddChild() {
    children_.push_back(std::make_unique<Scope>(this));
    return children_.back().get();
  }

 private:
  Scope* parent_;
  std::unordered_map<std::string, Symbol> symbols_;
  std::vector<std::unique_ptr<Scope>> children_;
};