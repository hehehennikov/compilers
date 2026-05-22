#pragma once

#include "base.hpp"

#include "misc.hpp"
#include "expressions.hpp"

namespace parser::ast::nodes {

/*
 * ex.: `mut x: i32`
 */
struct Parameter {
  std::unique_ptr<Pattern> pattern;
  std::unique_ptr<Type> type;
};

class FuncDecl : public Declaration {
 public:
  FuncDecl(std::string name,
           std::vector<std::unique_ptr<GenericParameter>> generics,
           std::vector<Parameter> params,
           std::unique_ptr<Type> return_type,
           std::vector<std::unique_ptr<WherePredicate>> where_clause,
           std::unique_ptr<RequiresClause> requires_clause,
           std::unique_ptr<BlockExpr> body,
           bool is_safe,
           common::SourceLocation l)
      : Declaration(l),
        name(std::move(name)),
        generics(std::move(generics)),
        params(std::move(params)),
        return_type(std::move(return_type)),
        where_clause(std::move(where_clause)),
        requires_clause(std::move(requires_clause)),
        body(std::move(body)),
        is_safe(is_safe) {}

 public: 
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public: 
  std::string name;
  std::vector<std::unique_ptr<GenericParameter>> generics;
  std::vector<Parameter> params;
  std::unique_ptr<Type> return_type; // nullptr for unit
  std::vector<std::unique_ptr<WherePredicate>> where_clause;
  std::unique_ptr<RequiresClause> requires_clause;
  std::unique_ptr<BlockExpr> body; // nullptr for abstract methods
  bool is_safe;
};

class StructDecl : public Declaration {
 public:
  StructDecl(std::string name,
             std::vector<std::unique_ptr<GenericParameter>> generics,
             std::vector<StructField> fields,
             std::vector<std::unique_ptr<WherePredicate>> where_clause,
             common::SourceLocation l)
      : Declaration(l),
        name(std::move(name)),
        generics(std::move(generics)),
        fields(std::move(fields)),
        where_clause(std::move(where_clause)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<GenericParameter>> generics;
  std::vector<StructField> fields;
  std::vector<std::unique_ptr<WherePredicate>> where_clause;
};

class EnumDecl : public Declaration {
 public:
  EnumDecl(std::string name,
           std::vector<std::unique_ptr<GenericParameter>> generics,
           std::vector<EnumVariant> variants,
           std::vector<std::unique_ptr<WherePredicate>> where_clause,
           common::SourceLocation l)
      : Declaration(l),
        name(std::move(name)),
        generics(std::move(generics)),
        variants(std::move(variants)),
        where_clause(std::move(where_clause)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<GenericParameter>> generics;
  std::vector<EnumVariant> variants;
  std::vector<std::unique_ptr<WherePredicate>> where_clause;
};

class TraitDecl : public Declaration {
 public:
  TraitDecl(std::string name,
            std::vector<std::unique_ptr<GenericParameter>> generics,
            std::vector<std::unique_ptr<FuncDecl>> methods,
            std::vector<std::unique_ptr<WherePredicate>> where_clause,
            common::SourceLocation l)
      : Declaration(l),
        name(std::move(name)),
        generics(std::move(generics)),
        methods(std::move(methods)),
        where_clause(std::move(where_clause)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<GenericParameter>> generics;
  std::vector<std::unique_ptr<FuncDecl>> methods;
  std::vector<std::unique_ptr<WherePredicate>> where_clause;
};

class ImplDecl : public Declaration {
 public:
  ImplDecl(std::unique_ptr<Path> trait_path,
           std::unique_ptr<Type> target_type,
           std::vector<std::unique_ptr<GenericParameter>> generics,
           std::vector<std::unique_ptr<FuncDecl>> methods,
           std::vector<std::unique_ptr<WherePredicate>> where_clause,
           common::SourceLocation l)
      : Declaration(l),
        trait_path(std::move(trait_path)),
        target_type(std::move(target_type)),
        generics(std::move(generics)),
        methods(std::move(methods)),
        where_clause(std::move(where_clause)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Path> trait_path;
  std::unique_ptr<Type> target_type;
  std::vector<std::unique_ptr<GenericParameter>> generics;
  std::vector<std::unique_ptr<FuncDecl>> methods;
  std::vector<std::unique_ptr<WherePredicate>> where_clause;
};

/*
 * ex: `using MyInt = i32;`
 */
class TypeAliasDecl : public Declaration {
 public:
  TypeAliasDecl(std::string name,
                std::vector<std::unique_ptr<GenericParameter>> generics,
                std::unique_ptr<Type> target_type,
                common::SourceLocation l)
      : Declaration(l),
        name(std::move(name)),
        generics(std::move(generics)),
        target_type(std::move(target_type)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<GenericParameter>> generics;
  std::unique_ptr<Type> target_type;
};

}  // namespace parser::ast::nodes