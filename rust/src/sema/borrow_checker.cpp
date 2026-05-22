#include "borrow_checker.hpp"

#include <common/diagnostic.hpp>
#include <parser/ast/nodes/all.hpp>

namespace sema {

using namespace parser::ast;

BorrowChecker::BorrowChecker() = default;

void BorrowChecker::Check(const std::vector<std::unique_ptr<nodes::Base>>& program) {
  for (const auto& node : program) {
    if (node != nullptr) {
      node->Accept(this);
    }
  }
}

void BorrowChecker::ValidateAccess(Symbol* sym, nodes::Base* node) {
  if (sym == nullptr) {
    return;
  }

  if (not states_.contains(sym)) {
    return;
  }

  const auto& state = states_.at(sym);

  // cannot use a value that has been moved
  if (state.status == OwnershipStatus::Moved) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error, 
        node->location,
        "Use of moved value: '" + sym->GetName() + "'"
    );
  }
}

void BorrowChecker::HandleMove(Symbol* source, nodes::Base* node) {
  if (source == nullptr) {
    return;
  }

  auto& state = states_[source];

  // cannot move out of a variable if it is currently borrowed
  if (state.status == OwnershipStatus::BorrowedImm || state.status == OwnershipStatus::BorrowedMut) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Cannot move out of '" + source->GetName() + "' because it is currently borrowed"
    );
    return;
  }

  // the source as dead
  state.status = OwnershipStatus::Moved;
}

void BorrowChecker::Visit(nodes::LetStmt* node) {
  if (node->initializer != nullptr) {
    node->initializer->Accept(this);

    // if RHS is an identifier and type is not copy, it's a move
    if (auto* ident = dynamic_cast<nodes::IdentExpr*>(node->initializer.get())) {
      auto type = ident->GetResolvedType();
      if (type != nullptr && !type->IsCopy()) {
        HandleMove(ident->symbol, node);
      }
    }
  }

  // initialize the state for the new variable(s) defined in the pattern
  if (node->pattern != nullptr) {
    node->pattern->Accept(this);
  }
}

void BorrowChecker::Visit(nodes::IdentExpr* node) {
  // check if we are trying to read from a moved variable
  ValidateAccess(node->symbol, node);
}

void BorrowChecker::Visit(nodes::BorrowExpr* node) {
  // logic for `&` and `&mut`
  if (auto* ident = dynamic_cast<nodes::IdentExpr*>(node->operand.get())) {
    if (ident->symbol == nullptr) {
      return;
    }

    auto& state = states_[ident->symbol];

    if (node->is_mutable) {
      if (state.status == OwnershipStatus::BorrowedImm || state.status == OwnershipStatus::BorrowedMut) {
        common::DiagnosticEngine::GetInstance().Report(
            common::DiagnosticLevel::Error,
            node->location,
            "Cannot borrow '" + ident->name + "' as mutable: already borrowed"
        );
      }
      state.status = OwnershipStatus::BorrowedMut;
    } else {
      // cannot borrow immutably if a mutable borrow exists
      if (state.status == OwnershipStatus::BorrowedMut) {
        common::DiagnosticEngine::GetInstance().Report(
            common::DiagnosticLevel::Error,
            node->location,
            "Cannot borrow '" + ident->name + "' as immutable: borrowed as mutable"
        );
      }
      state.status = OwnershipStatus::BorrowedImm;
      state.immutable_borrow_count++;
    }
  } else {
    // recursive check for complex operands (ex. &mut arr[0])
    node->operand->Accept(this);
  }
}

void BorrowChecker::Visit(nodes::AssignStmt* node) {
  // check RHS for moves
  if (node->rhs != nullptr) {
    node->rhs->Accept(this);
    if (auto* ident = dynamic_cast<nodes::IdentExpr*>(node->rhs.get())) {
      if (ident->symbol != nullptr && !ident->GetResolvedType()->IsCopy()) {
        HandleMove(ident->symbol, node);
      }
    }
  }

  // re-initializing a moved variable makes it 'Owned' again
  if (auto* ident = dynamic_cast<nodes::IdentExpr*>(node->lhs.get())) {
    if (ident->symbol != nullptr) {
      states_[ident->symbol].status = OwnershipStatus::Owned;
    }
  } else {
    node->lhs->Accept(this);
  }
}

void BorrowChecker::Visit(nodes::BlockExpr* node) {
  if (node->scope == nullptr) {
    return;
  }

  // traverse all statements within the block
  for (const auto& stmt : node->statements) {
    if (stmt != nullptr) {
      stmt->Accept(this);
    }
  }

  // handle the optional tail expression
  if (node->final_expression != nullptr) {
    node->final_expression->Accept(this);
  }

  // drop: cleanup symbols defined in this specific scope
  const auto& local_symbols = node->scope->GetLocalSymbols();

  for (const auto& [name, symbol_ptr] : local_symbols) {
    Symbol* sym = symbol_ptr.get();

    if (borrow_origins_.contains(sym)) {
      Symbol* owner = borrow_origins_[sym];

      if (states_.contains(owner)) {
        auto& owner_state = states_[owner];

        if (owner_state.status == OwnershipStatus::BorrowedMut) {
          owner_state.status = OwnershipStatus::Owned;
        } else if (owner_state.status == OwnershipStatus::BorrowedImm) {
          if (owner_state.immutable_borrow_count > 0) {
            owner_state.immutable_borrow_count--;
          }

          if (owner_state.immutable_borrow_count == 0) {
            owner_state.status = OwnershipStatus::Owned;
          }
        }
      }

      borrow_origins_.erase(sym);
    }

    states_.erase(sym);
  }
}

void BorrowChecker::Visit(nodes::FuncDecl* node) {
  states_.clear();

  for (auto& param : node->params) {
    if (param.pattern != nullptr) {
      param.pattern->Accept(this);
    }
  }

  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void BorrowChecker::Visit(nodes::MatchExpr* node) {
  if (node->value != nullptr) {
    node->value->Accept(this);
  }

  for (auto& arm : node->arms) {
    if (arm.pattern != nullptr) {
      arm.pattern->Accept(this);
    }
    if (arm.body != nullptr) {
      arm.body->Accept(this);
    }
  }
}


void BorrowChecker::Visit(nodes::BindingPattern* node) {
  if (node->symbol != nullptr) {
    states_[node->symbol].status = OwnershipStatus::Owned;
  }
}

void BorrowChecker::Visit(nodes::TuplePattern* node) {
  for (auto& el : node->elements) {
    if (el != nullptr) {
      el->Accept(this);
    }
  }
}

void BorrowChecker::Visit(nodes::StructPattern* node) {
  for (auto& field : node->fields) {
    if (field.pattern != nullptr) {
      field.pattern->Accept(this);
    }
  }
}

void BorrowChecker::Visit(nodes::UnaryExpr* node) { if (node->operand) node->operand->Accept(this); }
void BorrowChecker::Visit(nodes::BinaryExpr* node) {
  if (node->left) node->left->Accept(this);
  if (node->right) node->right->Accept(this);
}
void BorrowChecker::Visit(nodes::CallExpr* node) {
  if (node->callee) node->callee->Accept(this);
  for (auto& arg : node->arguments) if (arg) arg->Accept(this);
}
void BorrowChecker::Visit(nodes::MethodCallExpr* node) {
  if (node->object) node->object->Accept(this);
  for (auto& arg : node->arguments) if (arg) arg->Accept(this);
}
void BorrowChecker::Visit(nodes::IfExpr* node) {
  if (node->condition) node->condition->Accept(this);
  if (node->then_branch) node->then_branch->Accept(this);
  if (node->else_branch) node->else_branch->Accept(this);
}
void BorrowChecker::Visit(nodes::ReturnStmt* node) { if (node->value) node->value->Accept(this); }
void BorrowChecker::Visit(nodes::ExprStmt* node) { if (node->expr) node->expr->Accept(this); }
void BorrowChecker::Visit(nodes::WhileStmt* node) {
  if (node->condition) node->condition->Accept(this);
  if (node->body) node->body->Accept(this);
}
void BorrowChecker::Visit(nodes::ForInStmt* node) {
  if (node->iterable) node->iterable->Accept(this);
  if (node->pattern) node->pattern->Accept(this);
  if (node->body) node->body->Accept(this);
}

void BorrowChecker::Visit(nodes::ModuleDecl*) {}
void BorrowChecker::Visit(nodes::ImportDecl*) {}
void BorrowChecker::Visit(nodes::ExportDecl* node) { if (node->exported_item) node->exported_item->Accept(this); }
void BorrowChecker::Visit(nodes::ExternDecl*) {}
void BorrowChecker::Visit(nodes::Visibility*) {}
void BorrowChecker::Visit(nodes::Attribute*) {}
void BorrowChecker::Visit(nodes::Path*) {}
void BorrowChecker::Visit(nodes::Lifetime*) {}
void BorrowChecker::Visit(nodes::GenericParameter*) {}
void BorrowChecker::Visit(nodes::RequiresClause*) {}
void BorrowChecker::Visit(nodes::WherePredicate*) {}
void BorrowChecker::Visit(nodes::LiteralExpr*) {}
void BorrowChecker::Visit(nodes::SelfExpr*) {}
void BorrowChecker::Visit(nodes::UnitExpr*) {}
void BorrowChecker::Visit(nodes::SpaceshipExpr* n) { if (n->left) n->left->Accept(this); if (n->right) n->right->Accept(this); }
void BorrowChecker::Visit(nodes::CastExpr* n) { if (n->expr) n->expr->Accept(this); }
void BorrowChecker::Visit(nodes::MemberAccessExpr* n) { if (n->object) n->object->Accept(this); }
void BorrowChecker::Visit(nodes::IndexAccessExpr* n) { if (n->object) n->object->Accept(this); if (n->index) n->index->Accept(this); }
void BorrowChecker::Visit(nodes::LoopExpr* n) { if (n->body) n->body->Accept(this); }
void BorrowChecker::Visit(nodes::TryExpr* n) { if (n->expr) n->expr->Accept(this); }
void BorrowChecker::Visit(nodes::NewExpr* n) { for (auto& a : n->arguments) if (a) a->Accept(this); }
void BorrowChecker::Visit(nodes::LambdaExpr* n) { if (n->body) n->body->Accept(this); }
void BorrowChecker::Visit(nodes::SizeofExpr*) {}
void BorrowChecker::Visit(nodes::AlignofExpr*) {}
void BorrowChecker::Visit(nodes::DecltypeExpr*) {}
void BorrowChecker::Visit(nodes::ReflectExpr*) {}
void BorrowChecker::Visit(nodes::BreakStmt* n) { if (n->value) n->value->Accept(this); }
void BorrowChecker::Visit(nodes::ContinueStmt*) {}
void BorrowChecker::Visit(nodes::DeferStmt* n) { if (n->body) n->body->Accept(this); }
void BorrowChecker::Visit(nodes::StaticIfStmt* n) { if (n->condition) n->condition->Accept(this); }
void BorrowChecker::Visit(nodes::StructDecl*) {}
void BorrowChecker::Visit(nodes::EnumDecl*) {}
void BorrowChecker::Visit(nodes::ImplDecl* n) { for (auto& m : n->methods) if (m) m->Accept(this); }
void BorrowChecker::Visit(nodes::TraitDecl* n) { for (auto& m : n->methods) if (m) m->Accept(this); }
void BorrowChecker::Visit(nodes::TypeAliasDecl*) {}
void BorrowChecker::Visit(nodes::PrimitiveType*) {}
void BorrowChecker::Visit(nodes::ReferenceType*) {}
void BorrowChecker::Visit(nodes::PointerType*) {}
void BorrowChecker::Visit(nodes::ArrayType*) {}
void BorrowChecker::Visit(nodes::GenericType*) {}
void BorrowChecker::Visit(nodes::FunctionType*) {}
void BorrowChecker::Visit(nodes::PlaceholderType*) {}
void BorrowChecker::Visit(nodes::WildcardPattern*) {}
void BorrowChecker::Visit(nodes::LiteralPattern*) {}
void BorrowChecker::Visit(nodes::RangePattern*) {}
void BorrowChecker::Visit(nodes::ReferencePattern* n) { if (n->pattern) n->pattern->Accept(this); }

} // namespace sema