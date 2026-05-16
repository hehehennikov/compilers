#pragma once

#include "base.hpp"

namespace parser::ast::nodes {

struct Parameter {
  std::unique_ptr<PatternNode> pattern;
  std::unique_ptr<TypeNode> type;
};

class FuncDecl : public Declaration {
 public:
  FuncDecl(std::string name,
           std::vector<Parameter> params,
           std::unique_ptr<TypeNode> ret,
           std::unique_ptr<BlockExpr> body,
           common::SourceLocation l)
      : Declaration(std::move(l)),
        name(std::move(name)),
        params(std::move(params)),
        return_type(std::move(ret)),
        body(std::move(body)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<Parameter> params;
  std::unique_ptr<TypeNode> return_type;
  std::unique_ptr<BlockExpr> body;
};

class StructDecl : public Declaration {
 public:
  struct Field {
    std::string name;
    std::unique_ptr<TypeNode> type;
  };

 public:
  StructDecl(std::string name, std::vector<Field> f, common::SourceLocation l)
      : Declaration(std::move(l)), name(std::move(name)), fields(std::move(f)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<Field> fields;
};

struct EnumVariant {
  std::string name;
  std::vector<std::unique_ptr<TypeNode>> types;
};

class EnumDecl : public Declaration {
 public:
  EnumDecl(std::string name,
           std::vector<EnumVariant> vars,
           common::SourceLocation l)
      : Declaration(std::move(l)), name(std::move(name)), variants(std::move(vars)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<EnumVariant> variants;
};

class ImplDecl : public Declaration {
 public:
  ImplDecl(std::unique_ptr<TypeNode> trait,
           std::unique_ptr<TypeNode> target,
           std::vector<std::unique_ptr<FuncDecl>> m,
           common::SourceLocation l)
      : Declaration(std::move(l)),
        trait_name(std::move(trait)),
        target_type(std::move(target)),
        methods(std::move(m)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<TypeNode> trait_name;
  std::unique_ptr<TypeNode> target_type;
  std::vector<std::unique_ptr<FuncDecl>> methods;
};

class FunctionDecl : public Declaration {
 public:
  FunctionDecl(std::string name,
               std::vector<Parameter> params,
               std::unique_ptr<TypeNode> return_type,
               std::unique_ptr<BlockExpr> body,
               bool is_safe,
               bool is_async,
               common::SourceLocation l)
      : Declaration(std::move(l)),
        name(std::move(name)),
        params(std::move(params)),
        return_type(std::move(return_type)),
        body(std::move(body)),
        is_safe(is_safe),
        is_async(is_async) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<Parameter> params;

  std::unique_ptr<TypeNode> return_type; // may be nullptr

  std::unique_ptr<BlockExpr> body;

  bool is_safe;
  bool is_async;
};

class TraitDecl : public Declaration {
 public:
  TraitDecl(std::string name,
            std::vector<std::unique_ptr<FunctionDecl>> methods,
            common::SourceLocation l)
      : Declaration(std::move(l)),
        name(std::move(name)),
        methods(std::move(methods)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<FunctionDecl>> methods;
};

class TypeAliasDecl : public Declaration {
 public:
  TypeAliasDecl(std::string name, std::unique_ptr<TypeNode> type, common::SourceLocation l)
      : Declaration(std::move(l)), name(std::move(name)), target_type(std::move(type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::unique_ptr<TypeNode> target_type;
};

}  // namespace parser::ast::nodes