#pragma once

#include <string>
#include <vector>
#include <memory>

#include "fwd.hpp"

#include "base.hpp"
#include "misc.hpp"

namespace parser::ast::nodes {

/*
 * current module name:
 *   ex.: `module Network::Http;`
 */
class ModuleDecl : public ModuleItem {
 public:
  ModuleDecl(std::unique_ptr<Path> path, common::SourceLocation l)
      : ModuleItem(std::move(l)), path(std::move(path)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Path> path;
};

/*
 * import external name or entity:
 *   ex.: `import std::io;`
 */
class ImportDecl : public ModuleItem {
 public:
  ImportDecl(std::unique_ptr<Path> path, common::SourceLocation l)
      : ModuleItem(std::move(l)), path(std::move(path)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

  std::unique_ptr<Path> path;
  std::string alias; // `import std::io as input`
};

/*
 * makes entity visible for other modules
 *   ex.: `export func foo() -> int32 {}`
 */
class ExportDecl : public ModuleItem {
 public:
  ExportDecl(std::unique_ptr<Base> item, common::SourceLocation l)
      : ModuleItem(std::move(l)), exported_item(std::move(item)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Base> exported_item;
};

/*
 * external declarations block
 *   ex.: `extern "C" { func printf(...); }`
 */
class ExternDecl : public ModuleItem {
 public:
  ExternDecl(std::string abi,
             std::vector<std::unique_ptr<Declaration>> decls,
             common::SourceLocation l)
      : ModuleItem(std::move(l)), abi(std::move(abi)), declarations(std::move(decls)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string abi; // "C", "C++", "system"
  std::vector<std::unique_ptr<Declaration>> declarations;
};

enum class VisibilityLevel {
  Public, Private, Protected
};

class Visibility : public ModuleItem {
 public:
  Visibility(VisibilityLevel level,
             std::vector<std::unique_ptr<Base>> items,
             common::SourceLocation l)
      : ModuleItem(std::move(l)), level(level), items(std::move(items)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  VisibilityLevel level;
  std::vector<std::unique_ptr<Base>> items; // list with this visibility
};

} // namespace parser::ast::nodes