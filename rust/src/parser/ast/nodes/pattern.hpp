#pragma once

#include "base.hpp"

#include "misc.hpp"

#include <lexer/token.hpp>

namespace parser::ast::nodes {

class WildcardPattern : public Pattern {
 public:
  explicit WildcardPattern(common::SourceLocation l)
      : Pattern(l) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }
};

class BindingPattern : public Pattern {
 public:
  BindingPattern(std::string name, bool is_mutable, common::SourceLocation l)
      : Pattern(l), name(std::move(name)), is_mutable(is_mutable) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
  sema::Symbol* symbol = nullptr;
  bool is_mutable;
};

class LiteralPattern : public Pattern {
 public:
  LiteralPattern(lexer::TokenData value, common::SourceLocation l)
      : Pattern(l), value(std::move(value)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  lexer::TokenData value;
};

class RangePattern : public Pattern {
 public:
  RangePattern(std::unique_ptr<Expression> start,
               std::unique_ptr<Expression> end,
               bool is_inclusive,
               common::SourceLocation l)
      : Pattern(l), 
        start(std::move(start)), 
        end(std::move(end)), 
        is_inclusive(is_inclusive) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Expression> start;
  std::unique_ptr<Expression> end;
  bool is_inclusive; // .. or ..=
};

class TuplePattern : public Pattern {
 public:
  TuplePattern(std::vector<std::unique_ptr<Pattern>> elements, common::SourceLocation l)
      : Pattern(l), elements(std::move(elements)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::vector<std::unique_ptr<Pattern>> elements;
};

/*
 * ex.: Point { x, y: 10 }
 */
struct StructFieldPattern {
  std::string field_name;
  std::unique_ptr<Pattern> pattern;
};

class StructPattern : public Pattern {
 public:
  StructPattern(std::unique_ptr<Path> path,
                std::vector<StructFieldPattern> fields,
                common::SourceLocation l)
      : Pattern(l), path(std::move(path)), fields(std::move(fields)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Path> path; // struct name
  std::vector<StructFieldPattern> fields;
};

/*
 * ex.: match x { ref mut y => ... }
 */
class ReferencePattern : public Pattern {
 public:
  ReferencePattern(bool is_mutable,
                   std::unique_ptr<Pattern> pattern,
                   common::SourceLocation l)
      : Pattern(l),
        pattern(std::move(pattern)),
        is_mutable(is_mutable) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::unique_ptr<Pattern> pattern;
  bool is_mutable;
};

}  // namespace parser::ast::nodes