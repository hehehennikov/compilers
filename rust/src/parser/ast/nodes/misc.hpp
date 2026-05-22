#pragma once

#include "base.hpp"

#include "fwd.hpp"

namespace parser::ast::nodes {

struct PathSegment {
  std::string identifier;
  std::vector<std::unique_ptr<Type>> generics; // ex.: std::Vec<i32>
};

class Attribute : public Base {
 public:
  Attribute(std::string name,
            std::vector<std::unique_ptr<Expression>> arguments,
            common::SourceLocation l)
      : Base(l), name(std::move(name)), arguments(std::move(arguments)) {}

 public: 
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public: 
  std::string name;
  std::vector<std::unique_ptr<Expression>> arguments;
};

class Path : public Base {
 public:
  Path(std::vector<PathSegment> segments, bool is_absolute, common::SourceLocation l)
      : Base(l), segments(std::move(segments)), is_absolute(is_absolute) {}

 public: 
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public: 
  std::vector<PathSegment> segments;
  bool is_absolute; // begins with `::`
};

class Lifetime : public Base {
 public:
  Lifetime(std::string name, common::SourceLocation l)
      : Base(l), name(std::move(name)) {}

 public: 
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public: 
  std::string name; // "a", "static"
};

class GenericParameter : public Base {
 public:
  GenericParameter(std::string name,
                   std::vector<std::unique_ptr<Type>> bounds,
                   common::SourceLocation l)
      : Base(l), name(std::move(name)), bounds(std::move(bounds)) {}

 public: 
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  std::vector<std::unique_ptr<Type>> bounds; // ex.: `T: Clone + Display`
};

class RequiresClause : public Base {
 public:
  RequiresClause(std::unique_ptr<Expression> condition, common::SourceLocation l)
      : Base(std::move(l)), condition(std::move(condition)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> condition;
};

class WherePredicate : public Base {
 public:
  WherePredicate(std::unique_ptr<Type> type,
                 std::vector<std::unique_ptr<Type>> bounds,
                 common::SourceLocation l)
      : Base(l), target_type(std::move(type)), bounds(std::move(bounds)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Type> target_type;
  std::vector<std::unique_ptr<Type>> bounds;
};

struct MatchArm {
  std::unique_ptr<Pattern> pattern;
  std::unique_ptr<Expression> body;
  std::unique_ptr<Expression> guard;
};

struct StructField {
  std::string name;
  std::unique_ptr<Type> type;
  bool is_public;
};

struct EnumVariant {
  std::string name;
  std::vector<std::unique_ptr<Type>> types;
};

}  // namespace parser::ast::nodes