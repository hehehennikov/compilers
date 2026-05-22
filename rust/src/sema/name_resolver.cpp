#include "name_resolver.hpp"
#include <common/diagnostic.hpp>
#include <parser/ast/nodes/all.hpp>

namespace sema {

using namespace parser::ast;

NameResolver::NameResolver() {
  global_scope_ = std::make_unique<Scope>(nullptr);
  current_scope_ = global_scope_.get();

  RegisterBuiltIn("print", SymbolKind::Function);
}

void NameResolver::RegisterBuiltIn(std::string name, SymbolKind kind) {
  auto sym = std::make_unique<Symbol>(std::move(name), kind, nullptr);
  global_scope_->Define(std::move(sym));
}


void NameResolver::Resolve(const std::vector<std::unique_ptr<nodes::Base>>& program) {
  // pass 1: symbol collection
  // register all functions, structs, and aliases before looking into bodies
  for (const auto& node : program) {
    if (node != nullptr) {
      RegisterGlobalSymbol(node.get());
    }
  }

  // pass 2: full resolution
  for (const auto& node : program) {
    if (node != nullptr) {
      node->Accept(this);
    }
  }
}

/*
 * pass 1
 */
void NameResolver::RegisterGlobalSymbol(nodes::Base* node) {
  if (auto* func = dynamic_cast<nodes::FuncDecl*>(node)) {
    auto sym = std::make_unique<Symbol>(func->name, SymbolKind::Function, func);
    current_scope_->Define(std::move(sym));
  } else if (auto* str = dynamic_cast<nodes::StructDecl*>(node)) {
    auto sym = std::make_unique<Symbol>(str->name, SymbolKind::Struct, str);
    current_scope_->Define(std::move(sym));
  } else if (auto* alias = dynamic_cast<nodes::TypeAliasDecl*>(node)) {
    auto sym = std::make_unique<Symbol>(alias->name, SymbolKind::TypeAlias, alias);
    current_scope_->Define(std::move(sym));
  } else if (auto* exp = dynamic_cast<nodes::ExportDecl*>(node)) {
    // if it's an export, register the inner declaration
    RegisterGlobalSymbol(exp->exported_item.get());
  }
}

void NameResolver::EnterScope() {
  auto new_scope = std::make_unique<Scope>(current_scope_);
  current_scope_ = current_scope_->CreateChild();
}

void NameResolver::ExitScope() {
  Scope* parent = current_scope_->GetParent();

  if (parent != nullptr) {
    current_scope_ = current_scope_->GetParent();
  }
}

// --- PASS 2: Detailed Resolution ---

void NameResolver::Visit(nodes::ModuleDecl* node) {
  ///TODO:
  if (node->path != nullptr) {
    node->path->Accept(this);
  }
}

void NameResolver::Visit(nodes::ImportDecl* node) {
  ///TODO:
  if (node->path != nullptr) {
    node->path->Accept(this);
  }
}

void NameResolver::Visit(nodes::ExportDecl* node) {
  ///TODO:
  if (node->exported_item != nullptr) {
    node->exported_item->Accept(this);
  }
}

void NameResolver::Visit(nodes::ExternDecl* node) {
  ///TODO:
  for (const auto& decl : node->declarations) {
    if (decl != nullptr) {
      decl->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::Visibility* node) {
  for (const auto& item : node->items) {
    if (item != nullptr) {
      item->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::Attribute* node) {
  for (const auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::Lifetime* node) {
  auto sym = std::make_unique<Symbol>(node->name, SymbolKind::TypeAlias, node);
  current_scope_->Define(std::move(sym));
}

void NameResolver::Visit(nodes::GenericParameter* node) {
  auto sym = std::make_unique<Symbol>(node->name, SymbolKind::TypeAlias, node);
  current_scope_->Define(std::move(sym));

  for (const auto& bound : node->bounds) {
    if (bound != nullptr) {
      bound->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::RequiresClause* node) {
  if (node->condition != nullptr) {
    node->condition->Accept(this);
  }
}

void NameResolver::Visit(nodes::WherePredicate* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }

  for (const auto& bound : node->bounds) {
    if (bound != nullptr) {
      bound->Accept(this);
    }
  }
}


void NameResolver::Visit(nodes::FuncDecl* node) {
  // name is already registered in pass 1.
  EnterScope();

  // register generic type parameters (e.g., <T>) as types in local scope
  for (auto& gen_param : node->generics) {
    auto sym = std::make_unique<Symbol>(gen_param->name, SymbolKind::TypeAlias, gen_param.get());
    current_scope_->Define(std::move(sym));
  }

  // resolve parameters
  for (auto& param : node->params) {
    if (param.type != nullptr) {
      param.type->Accept(this);
    }
    // pattern might introduce new names
    if (param.pattern != nullptr) {
      param.pattern->Accept(this);
    }
  }

  if (node->return_type != nullptr) {
    node->return_type->Accept(this);
  }

  // resolve the function body
  if (node->body != nullptr) {
    node->body->Accept(this);
  }

  ExitScope();
}

void NameResolver::Visit(nodes::IdentExpr* node) {
  // single identifiers are resolved through the scope stack
  Symbol* sym = current_scope_->Lookup(node->name);

  if (sym == nullptr) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Identifier '" + node->name + "' not found"
    );
  } else {
    node->symbol = sym;
  }
}

void NameResolver::Visit(nodes::Path* node) {
  if (not node->segments.empty()) {
    Symbol* sym = current_scope_->Lookup(node->segments[0].identifier);

    if (sym == nullptr) {
      common::DiagnosticEngine::GetInstance().Report(
          common::DiagnosticLevel::Error,
          node->location,
          "Root of path '" + node->segments[0].identifier + "' not found"
      );
    }
  }
}

void NameResolver::Visit(nodes::SelfExpr* node) {
  Symbol* sym = current_scope_->Lookup("self");

  if (sym == nullptr) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Keyword 'self' is used outside of an implementation block or method"
    );
  } else {
    node->symbol = sym;
  }
}

void NameResolver::Visit(nodes::BorrowExpr* node) {
  if (node->operand != nullptr) {
    node->operand->Accept(this);
  }
}

void NameResolver::Visit(nodes::UnitExpr*) {
  // 'unit' contains no names to resolve
}

void NameResolver::Visit(nodes::UnaryExpr* node) {
  if (node->operand != nullptr) {
    node->operand->Accept(this);
  }
}

void NameResolver::Visit(nodes::BinaryExpr* node) {
  if (node->left != nullptr) {
    node->left->Accept(this);
  }
  if (node->right != nullptr) {
    node->right->Accept(this);
  }
}

void NameResolver::Visit(nodes::SpaceshipExpr* node) {
  if (node->left != nullptr) {
    node->left->Accept(this);
  }
  if (node->right != nullptr) {
    node->right->Accept(this);
  }
}

void NameResolver::Visit(nodes::CastExpr* node) {
  if (node->expr != nullptr) {
    node->expr->Accept(this);
  }
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }
}

void NameResolver::Visit(nodes::CallExpr* node) {
  if (node->callee != nullptr) {
    node->callee->Accept(this);
  }

  for (const auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::MethodCallExpr* node) {
  if (node->object != nullptr) {
    node->object->Accept(this);
  }

  for (const auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::MemberAccessExpr* node) {
  if (node->object != nullptr) {
    node->object->Accept(this);
  }
}

void NameResolver::Visit(nodes::IndexAccessExpr* node) {
  if (node->object != nullptr) {
    node->object->Accept(this);
  }
  if (node->index != nullptr) {
    node->index->Accept(this);
  }
}

void NameResolver::Visit(nodes::BlockExpr* node) {
  EnterScope();
  node->scope = current_scope_;
  for (const auto& stmt : node->statements) {
    if (stmt != nullptr) {
      stmt->Accept(this);
    }
  }
  if (node->final_expression != nullptr) {
    node->final_expression->Accept(this);
  }
  ExitScope();
}

void NameResolver::Visit(nodes::IfExpr* node) {
  if (node->condition != nullptr) {
    node->condition->Accept(this);
  }

  if (node->then_branch != nullptr) {
    node->then_branch->Accept(this);
  }

  if (node->else_branch != nullptr) {
    node->else_branch->Accept(this);
  }
}

void NameResolver::Visit(nodes::MatchExpr* node) {
  if (node->value != nullptr) {
    node->value->Accept(this);
  }

  // resolve each arm
  for (auto& arm : node->arms) {
    EnterScope();

    if (arm.pattern != nullptr) {
      arm.pattern->Accept(this);
    }

    if (arm.guard != nullptr) {
      arm.guard->Accept(this);
    }

    if (arm.body != nullptr) {
      arm.body->Accept(this);
    }

    ExitScope();
  }
}

void NameResolver::Visit(nodes::LoopExpr* node) {
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void NameResolver::Visit(nodes::TryExpr* node) {
  if (node->expr != nullptr) {
    node->expr->Accept(this);
  }
}

void NameResolver::Visit(nodes::NewExpr* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }

  // resolve arguments passed to the constructor
  for (const auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::LambdaExpr* node) {
  EnterScope();

  for (auto& param : node->params) {
    auto sym = std::make_unique<Symbol>(param.name, SymbolKind::Variable, &param);
    current_scope_->Define(std::move(sym));

    for (const auto& bound : param.bounds) {
      if (bound != nullptr) {
        bound->Accept(this);
      }
    }
  }

  if (node->body != nullptr) {
    node->body->Accept(this);
  }

  ExitScope();
}

void NameResolver::Visit(nodes::SizeofExpr* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }
}

void NameResolver::Visit(nodes::AlignofExpr* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }
}

void NameResolver::Visit(nodes::DecltypeExpr* node) {
  if (node->expr != nullptr) {
    node->expr->Accept(this);
  }
}

void NameResolver::Visit(nodes::ReflectExpr* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }
}

void NameResolver::Visit(nodes::ExprStmt* node) {
  if (node->expr != nullptr) {
    node->expr->Accept(this);
  }
}

void NameResolver::Visit(nodes::LetStmt* node) {
  // this ensures 'let x = x + 1' uses the 'x' from the outer scope.
  if (node->initializer != nullptr) {
    node->initializer->Accept(this);
  }

  // resolve the type annotation.
  if (node->type_ann != nullptr) {
    node->type_ann->Accept(this);
  }

  // register names from the pattern into the CURRENT scope.
  if (node->pattern != nullptr) {
    node->pattern->Accept(this);
  }
}

void NameResolver::Visit(nodes::AssignStmt* node) {
  if (node->lhs != nullptr) {
    node->lhs->Accept(this);
  }
  if (node->rhs != nullptr) {
    node->rhs->Accept(this);
  }
}

void NameResolver::Visit(nodes::WhileStmt* node) {
  if (node->condition != nullptr) {
    node->condition->Accept(this);
  }
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void NameResolver::Visit(nodes::ForInStmt* node) {
  EnterScope();

  if (node->iterable != nullptr) {
    node->iterable->Accept(this);
  }

  if (node->pattern != nullptr) {
    node->pattern->Accept(this);
  }

  if (node->body != nullptr) {
    node->body->Accept(this);
  }

  ExitScope();
}

void NameResolver::Visit(nodes::BreakStmt* node) {
  if (node->value != nullptr) {
    node->value->Accept(this);
  }
}

void NameResolver::Visit(nodes::ContinueStmt*) {
  // No identifiers to resolve in a simple 'continue'.
}

void NameResolver::Visit(nodes::ReturnStmt* node) {
  if (node->value != nullptr) {
    node->value->Accept(this);
  }
}

void NameResolver::Visit(nodes::DeferStmt* node) {
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void NameResolver::Visit(nodes::StaticIfStmt* node) {
  if (node->condition != nullptr) {
    node->condition->Accept(this);
  }
  if (node->then_branch != nullptr) {
    node->then_branch->Accept(this);
  }
  if (node->else_branch != nullptr) {
    node->else_branch->Accept(this);
  }
}

void NameResolver::Visit(nodes::StructDecl* node) {
  for (auto& field : node->fields) {
    if (field.type != nullptr) {
      field.type->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::EnumDecl* node) {
  for (auto& variant : node->variants) {
    for (auto& type : variant.types) {
      if (type != nullptr) {
        type->Accept(this);
      }
    }
  }
}

void NameResolver::Visit(nodes::ImplDecl* node) {
  EnterScope();

  auto self_sym = std::make_unique<Symbol>("self", SymbolKind::Variable, node);
  current_scope_->Define(std::move(self_sym));

  for (auto& gen : node->generics) {
    gen->Accept(this);
  }

  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }

  if (node->trait_path != nullptr) {
    node->trait_path->Accept(this);
  }

  for (auto& method : node->methods) {
    if (method != nullptr) {
      method->Accept(this);
    }
  }

  ExitScope();
}

void NameResolver::Visit(nodes::TraitDecl* node) {
  EnterScope();

  for (auto& gen : node->generics) {
    gen->Accept(this);
  }

  for (auto& method : node->methods) {
    if (method != nullptr) {
      method->Accept(this);
    }
  }

  ExitScope();
}

void NameResolver::Visit(nodes::TypeAliasDecl* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }
}

void NameResolver::Visit(nodes::ReferenceType* node) {
  if (node->lifetime != nullptr) {
    node->lifetime->Accept(this);
  }
  if (node->base != nullptr) {
    node->base->Accept(this);
  }
}

void NameResolver::Visit(nodes::PointerType* node) {
  if (node->base != nullptr) {
    node->base->Accept(this);
  }
}

void NameResolver::Visit(nodes::ArrayType* node) {
  if (node->base != nullptr) {
    node->base->Accept(this);
  }
  if (node->size_expr != nullptr) {
    node->size_expr->Accept(this);
  }
}

void NameResolver::Visit(nodes::GenericType* node) {
  if (node->path != nullptr) {
    node->path->Accept(this);
  }

  for (auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::FunctionType* node) {
  for (auto& param : node->params) {
    if (param != nullptr) {
      param->Accept(this);
    }
  }
  if (node->return_type != nullptr) {
    node->return_type->Accept(this);
  }
}

void NameResolver::Visit(nodes::PlaceholderType*) {
  // nothing to resolve here
}

void NameResolver::Visit(nodes::LiteralExpr*) {
  // nothing to resolve here
}

void NameResolver::Visit(nodes::WildcardPattern*) {
  // nothing to resolve here
}

void NameResolver::Visit(nodes::BindingPattern* node) {
  // create a new symbol for the variable.
  auto sym = std::make_unique<Symbol>(node->name, SymbolKind::Variable, node);

  node->symbol = sym.get();

  // define it in the current scope.
  current_scope_->Define(std::move(sym));
}

void NameResolver::Visit(nodes::LiteralPattern*) {
  // nothing to resolve here
}

void NameResolver::Visit(nodes::RangePattern* node) {
  if (node->start != nullptr) {
    node->start->Accept(this);
  }

  if (node->end != nullptr) {
    node->end->Accept(this);
  }
}

void NameResolver::Visit(nodes::TuplePattern* node) {
  for (auto& element : node->elements) {
    if (element != nullptr) {
      element->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::StructPattern* node) {
  if (node->path != nullptr) {
    node->path->Accept(this);
  }

  for (auto& field : node->fields) {
    if (field.pattern != nullptr) {
      field.pattern->Accept(this);
    }
  }
}

void NameResolver::Visit(nodes::ReferencePattern* node) {
  if (node->pattern != nullptr) {
    node->pattern->Accept(this);
  }
}

void NameResolver::Visit(nodes::PrimitiveType* node) {}

} // namespace sema