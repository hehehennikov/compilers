#pragma once

#include "base.hpp"

#include <lexer/token.hpp>

namespace parser::ast::nodes {

class WildcardPattern : public PatternNode {
 public:
  using PatternNode::PatternNode;

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }
};

class BindingPattern : public PatternNode {
 public:
  BindingPattern(std::string name, common::SourceLocation l)
      : PatternNode(std::move(l)), name(std::move(name)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string name;
};

class LiteralPattern : public PatternNode {
 public:
  LiteralPattern(lexer::TokenData val, common::SourceLocation l)
      : PatternNode(std::move(l)), value(std::move(val)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  lexer::TokenData value;
};

class TuplePattern : public PatternNode {
 public:
  TuplePattern(std::vector<std::unique_ptr<PatternNode>> el, common::SourceLocation l)
      : PatternNode(std::move(l)), elements(std::move(el)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::vector<std::unique_ptr<PatternNode>> elements;
};

class StructPattern : public PatternNode {
 public:
  struct FieldPattern {
    std::string name;
    std::unique_ptr<PatternNode> pattern;
  };

 public:
  StructPattern(std::string name,
                std::vector<FieldPattern> fields,
                common::SourceLocation l)
      : PatternNode(std::move(l)),
        struct_name(std::move(name)),
        fields(std::move(fields)) {}

 public:
  void Accept(visitor::IVisitor* v) override {
    v->Visit(this);
  }

 public:
  std::string struct_name;
  std::vector<FieldPattern> fields;
};

}  // namespace parser::ast::nodes