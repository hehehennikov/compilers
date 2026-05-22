#include "print.hpp"

#include <parser/ast/nodes/all.hpp>

#include <magic_enum/magic_enum.hpp>

namespace parser::ast::visitor {

using namespace nodes;

Print::Print(std::ostream& out)
    : out_(out) {}

void Print::Run(Base* node) {
  if (node) {
    node->Accept(this);
  }
}

void Print::Indent() const {
  for (std::uint32_t i = 0; i < indent_level_; ++i) {
    out_ << "  ";
  }
}

void Print::VisitChild(Base* node) {
  if (node) {
    indent_level_++;
    node->Accept(this);
    indent_level_--;
  } else {
    indent_level_++;
    Indent(); out_ << "<null>\n";
    indent_level_--;
  }
}

void Print::PrintLocation(const common::SourceLocation& loc) const {
  out_ << "Current location: " << loc.file_name <<
          ":" << loc.line << ":" << loc.column << "\n";
}

void Print::PrintValue(const lexer::TokenData& data) const {
  std::visit([this]<typename U>(U&& arg) {
    using T = std::decay_t<U>;
    if constexpr (std::is_same_v<T, std::monostate>) {
      out_ << "None";
    } else if constexpr (std::is_same_v<T, std::string>) {
      out_ << "\"" << arg << "\"";
    } else if constexpr (std::is_same_v<T, bool>) {
      out_ << std::boolalpha << arg;
    } else {
      out_ << arg;
    }
  }, data);
}

void Print::Visit(ModuleDecl* node) {
  Indent();
  out_ << "ModuleDecl: ";

  node->path->Accept(this);

  out_ << "\n";
}

void Print::Visit(ImportDecl* node) {
  Indent();
  out_ << "ImportDecl: ";

  if (node->path) {
    node->path->Accept(this);
  }
  if (not node->alias.empty()) {
    out_ << " as " << node->alias;
  }
  out_ << "\n";
}

void Print::Visit(ExportDecl* node) {
  Indent();
  out_ << "ExportDecl\n";

  VisitChild(node->exported_item.get());
}

void Print::Visit(ExternDecl* node) {
  Indent();
  out_ << "ExternDecl (ABI: \"" << node->abi << "\")\n";

  for (auto& decl : node->declarations) {
    VisitChild(decl.get());
  }
}

void Print::Visit(Visibility* node) {
  Indent();
  out_ << "VisibilityBlock (Level: " << (int)node->level << ")\n";

  for (auto& item : node->items) {
    VisitChild(item.get());
  }
}

void Print::Visit(Attribute* node) {
  Indent();
  out_ << "Attribute: #" << node->name << "\n";

  if (not node->arguments.empty()) {
    for (auto& arg : node->arguments) {
      VisitChild(arg.get());
    }
  }
}

void Print::Visit(Path* node) {
  if (node->is_absolute) {
    out_ << "::";
  }

  for (std::size_t i = 0; i < node->segments.size(); ++i) {
    const auto& segment = node->segments[i];
    out_ << segment.identifier;

    if (not segment.generics.empty()) {
      out_ << "<";
      for (std::size_t j = 0; j < segment.generics.size(); ++j) {
        segment.generics[j]->Accept(this);
        if (j < segment.generics.size() - 1) {
          out_ << ", ";
        }
      }
      out_ << ">";
    }

    if (i < node->segments.size() - 1) {
      out_ << "::";
    }
  }
}

void Print::Visit(Lifetime* node) {
  out_ << "'" << node->name;
}

void Print::Visit(GenericParameter* node) {
  Indent();
  out_ << "GenericParam: " << node->name << "\n";

  if (not node->bounds.empty()) {
    indent_level_++;

    Indent();
    out_ << "Bounds:\n";

    for (auto& bound : node->bounds) {
      VisitChild(bound.get());
    }

    indent_level_--;
  }
}

void Print::Visit(RequiresClause* node) {
  Indent();
  out_ << "RequiresClause:\n";

  VisitChild(node->condition.get());
}

void Print::Visit(WherePredicate* node) {
  Indent();
  out_ << "WherePredicate:\n";

  indent_level_++;

  Indent();
  out_ << "TargetType:\n";

  VisitChild(node->target_type.get());

  Indent();
  out_ << "Bounds:\n";

  for (auto& bound : node->bounds) {
    VisitChild(bound.get());
  }

  indent_level_--;
}

void Print::Visit(LiteralExpr* node) {
  Indent();
  out_ << "LiteralExpr: ";

  PrintValue(node->value);

  out_ << "\n";
}

void Print::Visit(IdentExpr* node) {
  Indent();
  out_ << "IdentExpr: " << node->name << "\n";
}

void Print::Visit(SelfExpr* node) {
  Indent();
  out_ << "SelfExpr (self)\n";
}

void Print::Visit(BorrowExpr* node) {
  Indent();
  out_ << "BorrowExpr (&" << (node->is_mutable ? "mut" : "") << ")\n";

  VisitChild(node->operand.get());
}

template <typename E>
auto GetEnumName(E e) {
  return magic_enum::enum_name(e);
}

void Print::Visit(UnitExpr* node) {
  Indent();
  out_ << "UnitExpr ()\n";
}

void Print::Visit(UnaryExpr* node) {
  Indent();
  out_ << "UnaryExpr (op: " << GetEnumName(node->op) << ")\n";

  VisitChild(node->operand.get());
}

void Print::Visit(BinaryExpr* node) {
  Indent();
  out_ << "BinaryExpr (op: " << GetEnumName(node->op) << ")\n";

  indent_level_++;

  Indent();
  out_ << "LHS:\n";
  VisitChild(node->left.get());
  Indent();
  out_ << "RHS:\n";
  VisitChild(node->right.get());

  indent_level_--;
}

void Print::Visit(SpaceshipExpr* node) {
  Indent();
  out_ << "SpaceshipExpr (<=>)\n";

  indent_level_++;

  VisitChild(node->left.get());
  VisitChild(node->right.get());

  indent_level_--;
}

void Print::Visit(CastExpr* node) {
  Indent();
  out_ << "CastExpr (as)\n";

  indent_level_++;

  Indent();
  out_ << "Source:\n";
  VisitChild(node->expr.get());
  Indent();
  out_ << "TargetType:\n";
  VisitChild(node->target_type.get());

  indent_level_--;
}

void Print::Visit(CallExpr* node) {
  Indent();
  out_ << "CallExpr\n";

  indent_level_++;

  Indent();
  out_ << "Callee:\n";
  VisitChild(node->callee.get());
  Indent();
  out_ << "Arguments (" << node->arguments.size() << "):\n";

  for (auto& arg : node->arguments) {
    VisitChild(arg.get());
  }

  indent_level_--;
}

void Print::Visit(MethodCallExpr* node) {
  Indent();
  out_ << "MethodCallExpr (." << node->method_name << ")\n";

  indent_level_++;

  Indent();
  out_ << "Object:\n";
  VisitChild(node->object.get());
  Indent();
  out_ << "Arguments:\n";

  for (auto& arg : node->arguments) {
    VisitChild(arg.get());
  }

  indent_level_--;
}

void Print::Visit(MemberAccessExpr* node) {
  Indent();
  out_ << "MemberAccessExpr (." << node->member_name << ")\n";

  VisitChild(node->object.get());
}

void Print::Visit(IndexAccessExpr* node) {
  Indent();
  out_ << "IndexAccessExpr ([])\n";

  indent_level_++;

  Indent();
  out_ << "Object:\n";
  VisitChild(node->object.get());
  Indent();
  out_ << "Index:\n";
  VisitChild(node->index.get());

  indent_level_--;
}

void Print::Visit(BlockExpr* node) {
  Indent();
  out_ << "BlockExpr\n";

  indent_level_++;

  for (auto& stmt : node->statements) {
    VisitChild(stmt.get());
  }

  if (node->final_expression) {
    Indent();
    out_ << "Result (Tail Expression):\n";

    VisitChild(node->final_expression.get());
  }

  indent_level_--;
}

void Print::Visit(IfExpr* node) {
  Indent();
  out_ << "IfExpr\n";

  indent_level_++;

  Indent();
  out_ << "Condition:\n";
  VisitChild(node->condition.get());
  Indent();
  out_ << "Then:\n";
  VisitChild(node->then_branch.get());

  if (node->else_branch) {
    Indent();
    out_ << "Else:\n";
    VisitChild(node->else_branch.get());
  }
  indent_level_--;
}

void Print::Visit(MatchExpr* node) {
  Indent();
  out_ << "MatchExpr\n";

  indent_level_++;

  Indent();
  out_ << "Value:\n";
  VisitChild(node->value.get());

  for (std::size_t i = 0; i < node->arms.size(); ++i) {
    Indent();
    out_ << "Arm #" << i << ":\n";

    indent_level_++;

    Indent();
    out_ << "Pattern:\n";
    VisitChild(node->arms[i].pattern.get());

    if (node->arms[i].guard) {
      Indent();
      out_ << "Guard (if):\n";
      VisitChild(node->arms[i].guard.get());
    }

    Indent();
    out_ << "Body:\n";
    VisitChild(node->arms[i].body.get());

    indent_level_--;
  }
  indent_level_--;
}

void Print::Visit(LoopExpr* node) {
  Indent();
  out_ << "LoopExpr\n";

  VisitChild(node->body.get());
}

void Print::Visit(TryExpr* node) {
  Indent();
  out_ << "TryExpr (?)\n";

  VisitChild(node->expr.get());
}

void Print::Visit(NewExpr* node) {
  Indent();
  out_ << "NewExpr (Heap Allocation)\n";

  indent_level_++;

  Indent();
  out_ << "Type:\n";
  VisitChild(node->target_type.get());

  if (not node->arguments.empty()) {
    Indent();
    out_ << "Constructor Args:\n";
    for (auto& arg : node->arguments) {
      VisitChild(arg.get());
    }
  }

  indent_level_--;
}

void Print::Visit(LambdaExpr* node) {
  Indent();
  out_ << "LambdaExpr (move: " << (node->is_move ? "true" : "false") << ")\n";

  indent_level_++;

  Indent();
  out_ << "Parameters:\n";
  for (auto& p : node->params) {
    Indent(); out_ << "| " << p.name << "\n";
  }
  Indent();
  out_ << "Body:\n";
  VisitChild(node->body.get());

  indent_level_--;
}

void Print::Visit(SizeofExpr* node) {
  Indent();
  out_ << "SizeofExpr\n";

  VisitChild(node->target_type.get());
}

void Print::Visit(AlignofExpr* node) {
  Indent(); out_ << "AlignofExpr\n";
  VisitChild(node->target_type.get());
}

void Print::Visit(DecltypeExpr* node) {
  Indent();
  out_ << "DecltypeExpr\n";

  VisitChild(node->expr.get());
}

void Print::Visit(ReflectExpr* node) {
  Indent();
  out_ << "ReflectExpr (Static Introspection)\n";

  VisitChild(node->target_type.get());
}

void Print::Visit(ExprStmt* node) {
  Indent();
  out_ << "ExprStmt (;)\n";

  VisitChild(node->expr.get());
}

void Print::Visit(LetStmt* node) {
  Indent();
  out_ << "LetStmt (mutable: " << (node->is_mutable ? "true" : "false") << ")\n";

  indent_level_++;

  Indent();
  out_ << "Pattern:\n";
  VisitChild(node->pattern.get());

  if (node->type_ann) {
    Indent();
    out_ << "ExplicitType:\n";
    VisitChild(node->type_ann.get());
  }
  if (node->initializer) {
    Indent();
    out_ << "Initializer:\n";
    VisitChild(node->initializer.get());
  }

  indent_level_--;
}

void Print::Visit(AssignStmt* node) {
  Indent();
  out_ << "AssignStmt (op: " << GetEnumName(node->op) << ")\n";

  indent_level_++;

  Indent();
  out_ << "LHS:\n";
  VisitChild(node->lhs.get());
  Indent();
  out_ << "RHS:\n";
  VisitChild(node->rhs.get());

  indent_level_--;
}

void Print::Visit(WhileStmt* node) {
  Indent();
  out_ << "WhileStmt\n";

  indent_level_++;

  Indent();
  out_ << "Condition:\n";
  VisitChild(node->condition.get());
  Indent();
  out_ << "Body:\n";
  VisitChild(node->body.get());

  indent_level_--;
}

void Print::Visit(ForInStmt* node) {
  Indent();
  out_ << "ForInStmt\n";

  indent_level_++;

  Indent();
  out_ << "Pattern:\n";
  VisitChild(node->pattern.get());
  Indent();
  out_ << "Iterable:\n";
  VisitChild(node->iterable.get());
  Indent();
  out_ << "Body:\n";
  VisitChild(node->body.get());

  indent_level_--;
}

void Print::Visit(BreakStmt* node) {
  Indent();
  out_ << "BreakStmt\n";

  if (node->value) {
    Indent();
    out_ << "Value:\n";
    VisitChild(node->value.get());
  }
}

void Print::Visit(ContinueStmt* node) {
  Indent();
  out_ << "ContinueStmt\n";
}

void Print::Visit(ReturnStmt* node) {
  Indent();
  out_ << "ReturnStmt\n";

  if (node->value) {
    VisitChild(node->value.get());
  }
}

void Print::Visit(DeferStmt* node) {
  Indent();
  out_ << "DeferStmt (Cleanup at scope end)\n";

  VisitChild(node->body.get());
}

void Print::Visit(StaticIfStmt* node) {
  Indent();
  out_ << "StaticIfStmt (Compile-time if)\n";
  indent_level_++;

  Indent();
  out_ << "Condition:\n";
  VisitChild(node->condition.get());
  Indent();
  out_ << "Then:\n";
  VisitChild(node->then_branch.get());

  if (node->else_branch) {
    Indent();
    out_ << "Else:\n";
    VisitChild(node->else_branch.get());
  }

  indent_level_--;
}

void Print::Visit(FuncDecl* node) {
  Indent(); out_ << "FuncDecl: " << node->name;
  if (node->is_safe) {
    out_ << " [safe]";
  }
  out_ << "\n";

  indent_level_++;
  if (not node->generics.empty()) {
    Indent(); out_ << "Generics:\n";

    for (auto& g : node->generics) {
      VisitChild(g.get());
    }
  }

  Indent(); out_ << "Parameters:\n";

  for (auto& param : node->params) {
    indent_level_++;

    Indent(); out_ << "Param:\n";

    indent_level_++;

    Indent(); out_ << "Pattern:\n"; VisitChild(param.pattern.get());
    Indent(); out_ << "Type:\n"; VisitChild(param.type.get());

    indent_level_ -= 2;
  }

  if (node->return_type) {
    Indent(); out_ << "ReturnType:\n"; VisitChild(node->return_type.get());
  }

  if (not node->where_clause.empty()) {
    Indent(); out_ << "WhereClause:\n";

    for (auto& w : node->where_clause) {
      VisitChild(w.get());
    }
  }

  if (node->requires_clause) {
    VisitChild(node->requires_clause.get());
  }

  if (node->body) {
    Indent(); out_ << "Body:\n"; VisitChild(node->body.get());
  } else {
    Indent(); out_ << "Body: <abstract/extern>\n";
  }
  indent_level_--;
}

void Print::Visit(StructDecl* node) {
  Indent(); out_ << "StructDecl: " << node->name << "\n";

  indent_level_++;

  for (auto& field : node->fields) {
    Indent(); out_ << "Field: " << field.name
                   << (field.is_public ? " [public]" : " [private]") << "\n";
    VisitChild(field.type.get());
  }

  indent_level_--;
}

void Print::Visit(EnumDecl* node) {
  Indent(); out_ << "EnumDecl: " << node->name << "\n";

  indent_level_++;

  for (auto& variant : node->variants) {
    Indent(); out_ << "Variant: " << variant.name << "\n";

    if (not variant.types.empty()) {
      indent_level_++;

      for (auto& t : variant.types) {
        VisitChild(t.get());
      }

      indent_level_--;
    }
  }
  indent_level_--;
}

void Print::Visit(ImplDecl* node) {
  Indent(); out_ << "ImplDecl\n";

  indent_level_++;

  if (node->trait_path) {
    Indent(); out_ << "Trait:\n"; VisitChild(node->trait_path.get());
    Indent(); out_ << "For Target:\n";
  } else {
    Indent(); out_ << "Inherent Impl for:\n";
  }

  VisitChild(node->target_type.get());

  Indent(); out_ << "Methods:\n";

  for (auto& method : node->methods) {
    VisitChild(method.get());
  }

  indent_level_--;
}

void Print::Visit(TraitDecl* node) {
  Indent(); out_ << "TraitDecl: " << node->name << "\n";

  indent_level_++;

  for (auto& method : node->methods) {
    VisitChild(method.get());
  }

  indent_level_--;
}

void Print::Visit(TypeAliasDecl* node) {
  Indent(); out_ << "TypeAlias: " << node->name << "\n";

  VisitChild(node->target_type.get());
}

void Print::Visit(PrimitiveType* node) {
  Indent(); out_ << "Type: Primitive (token_kind: " << (int)node->kind << ")\n";
}

void Print::Visit(ReferenceType* node) {
  Indent(); out_ << "Type: Reference (&" << (node->is_mutable ? "mut " : "") << ")\n";

  if (node->lifetime) {
    indent_level_++;

    VisitChild(node->lifetime.get());

    indent_level_--;
  }

  VisitChild(node->base.get());
}

void Print::Visit(PointerType* node) {
  std::string kind_str;
  switch (node->kind) {
    case PointerType::Kind::RawConst:
      kind_str = "*const";
      break;
    case PointerType::Kind::RawMut:
      kind_str = "*mut";
      break;
    case PointerType::Kind::Box:
      kind_str = "Box";
      break;
    case PointerType::Kind::Unique:
      kind_str = "Unique";
      break;
  }

  Indent(); out_ << "Type: Pointer (" << kind_str << ")\n";

  VisitChild(node->base.get());
}

void Print::Visit(ArrayType* node) {
  Indent(); out_ << "Type: Array\n";

  indent_level_++;

  Indent(); out_ << "Base:\n"; VisitChild(node->base.get());
  Indent(); out_ << "Size:\n"; VisitChild(node->size_expr.get());

  indent_level_--;
}

void Print::Visit(GenericType* node) {
  Indent(); out_ << "Type: Generic Application\n";

  indent_level_++;

  Indent(); out_ << "Template: "; VisitChild(node->path.get()); out_ << '\n';
  Indent(); out_ << "Arguments:\n";

  for (auto& arg : node->arguments) {
    VisitChild(arg.get());
  }

  indent_level_--;
}

void Print::Visit(FunctionType* node) {
  Indent(); out_ << "Type: Function Pointer (fn)\n";

  indent_level_++;

  Indent(); out_ << "Params:\n";

  for (auto& p : node->params) {
    VisitChild(p.get());
  }

  if (node->return_type) {
    Indent(); out_ << "Returns:\n"; VisitChild(node->return_type.get());
  }

  indent_level_--;
}

void Print::Visit(PlaceholderType* node) {
  Indent(); out_ << "Type: Placeholder (_)\n";
}

void Print::Visit(WildcardPattern* node) {
  Indent(); out_ << "Pattern: Wildcard (_)\n";
}

void Print::Visit(BindingPattern* node) {
  Indent(); out_ << "Pattern: Binding (name: \"" << node->name << "\", mut: "
                 << (node->is_mutable ? "true" : "false") << ")\n";
}

void Print::Visit(LiteralPattern* node) {
  Indent(); out_ << "Pattern: Literal (";

  PrintValue(node->value);

  out_ << ")\n";
}

void Print::Visit(RangePattern* node) {
  Indent(); out_ << "Pattern: Range (inclusive: "
                 << (node->is_inclusive ? "true" : "false") << ")\n";

  indent_level_++;

  Indent(); out_ << "Start:\n"; VisitChild(node->start.get());
  Indent(); out_ << "End:\n";   VisitChild(node->end.get());

  indent_level_--;
}

void Print::Visit(TuplePattern* node) {
  Indent(); out_ << "Pattern: Tuple (" << node->elements.size() << " elements)\n";

  indent_level_++;

  for (auto& element : node->elements) {
    VisitChild(element.get());
  }

  indent_level_--;
}

void Print::Visit(StructPattern* node) {
  Indent(); out_ << "Pattern: Struct (type: ";
  if (node->path) {
    node->path->Accept(this);
  }
  out_ << ")\n";

  indent_level_++;

  for (auto& field : node->fields) {
    Indent(); out_ << "Field: " << field.field_name << "\n";
    VisitChild(field.pattern.get());
  }

  indent_level_--;
}

void Print::Visit(ReferencePattern* node) {
  Indent(); out_ << "Pattern: Reference (&" << (node->is_mutable ? "mut " : "") << ")\n";

  VisitChild(node->pattern.get());
}

}  // namespace parser::ast::visitor