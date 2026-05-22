#include "type_checker.hpp"

#include <common/diagnostic.hpp>
#include <parser/ast/nodes/all.hpp>

#include "type.hpp"

namespace sema {

using namespace parser::ast;

TypeChecker::TypeChecker() = default;

void TypeChecker::Check(const std::vector<std::unique_ptr<nodes::Base>>& program) {
  for (const auto& node : program) {
    if (node != nullptr) {
      node->Accept(this);
    }
  }
}

std::shared_ptr<Type> TypeChecker::ResolveSyntacticType(nodes::Type* node) {
  if (node == nullptr) {
    return std::make_shared<PrimitiveType>(PrimitiveType::Kind::Unit);
  }

  if (nodes::PrimitiveType* prim = dynamic_cast<nodes::PrimitiveType*>(node)) {
    return std::make_shared<PrimitiveType>(prim->kind);
  }

  if (auto* ref = dynamic_cast<nodes::ReferenceType*>(node)) {
    auto base = ResolveSyntacticType(ref->base.get());
    return std::make_shared<sema::ReferenceType>(base, ref->is_mutable);
  }

  if (auto* ptr = dynamic_cast<nodes::PointerType*>(node)) {
    auto base = ResolveSyntacticType(ptr->base.get());
    auto kind = static_cast<sema::PointerType::Kind>(ptr->kind);
    return std::make_shared<sema::PointerType>(kind, base);
  }

  return nullptr;
}

void TypeChecker::Visit(nodes::LiteralExpr* node) {
  if (std::holds_alternative<std::int8_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Int8);
  } else if (std::holds_alternative<std::int16_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Int16);
  } else if (std::holds_alternative<std::int32_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Int32);
  } else if (std::holds_alternative<std::int64_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Int64);
  } else if (std::holds_alternative<std::uint8_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::UInt8);
  } else if (std::holds_alternative<std::uint16_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::UInt16);
  } else if (std::holds_alternative<std::uint32_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::UInt32);
  } else if (std::holds_alternative<std::uint64_t>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::UInt64);
  } else if (std::holds_alternative<float>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Float32);
  } else if (std::holds_alternative<double>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Float64);
  } else if (std::holds_alternative<bool>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Bool);
  } else if (std::holds_alternative<std::string>(node->value)) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::String);
  } else {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Unit);
  }

  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::IdentExpr* node) {
  if (node->symbol != nullptr) {
    last_type_ = node->symbol->GetType();
    node->resolved_type = last_type_;
  } else {
    last_type_ = nullptr;
  }
}

void TypeChecker::Visit(nodes::BinaryExpr* node) {
  node->left->Accept(this);
  auto left_type = last_type_;

  node->right->Accept(this);
  auto right_type = last_type_;

  if (left_type == nullptr || right_type == nullptr || !left_type->IsEqualTo(right_type.get())) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Type mismatch in binary expression"
    );
    last_type_ = nullptr;
    return;
  }

  bool is_comparison = (node->op == lexer::TokenType::EqEq ||
                        node->op == lexer::TokenType::Less || 
                        node->op == lexer::TokenType::Greater);

  if (is_comparison) {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Bool);
  } else {
    last_type_ = left_type;
  }

  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::CastExpr* node) {
  node->expr->Accept(this);
  
  auto target = ResolveSyntacticType(node->target_type.get());
  
  last_type_ = target;
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::BlockExpr* node) {
  for (const auto& stmt : node->statements) {
    if (stmt != nullptr) {
      stmt->Accept(this);
    }
  }

  if (node->final_expression != nullptr) {
    node->final_expression->Accept(this);
    node->resolved_type = last_type_;
  } else {
    last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Unit);
    node->resolved_type = last_type_;
  }
}

void TypeChecker::Visit(nodes::LetStmt* node) {
  std::shared_ptr<Type> final_type = nullptr;

  if (node->type_ann != nullptr) {
    final_type = ResolveSyntacticType(node->type_ann.get());
  } else if (node->initializer != nullptr) {
    node->initializer->Accept(this);
    final_type = last_type_;
  }

  if (final_type == nullptr) {
    common::DiagnosticEngine::GetInstance().Report(
      common::DiagnosticLevel::Error, node->location, "Could not infer type");
    return;
  }

  node->pattern->SetResolvedType(final_type);

  if (auto* bind = dynamic_cast<nodes::BindingPattern*>(node->pattern.get())) {
    if (bind->symbol != nullptr) {
      bind->symbol->SetType(final_type);
    }
  }
}

void TypeChecker::Visit(nodes::AssignStmt* node) {
  node->lhs->Accept(this);
  auto lhs_type = last_type_;

  node->rhs->Accept(this);
  auto rhs_type = last_type_;

  if (lhs_type != nullptr && rhs_type != nullptr && !lhs_type->IsEqualTo(rhs_type.get())) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Cannot assign value of type " + rhs_type->ToString() + " to type " + lhs_type->ToString()
    );
  }
}

void TypeChecker::Visit(nodes::ReturnStmt* node) {
  std::shared_ptr<Type> actual_ret = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Unit);
  
  if (node->value != nullptr) {
    node->value->Accept(this);
    actual_ret = last_type_;
  }

  if (current_expected_return_type_ != nullptr && not actual_ret->IsEqualTo(current_expected_return_type_.get())) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Return type mismatch"
    );
  }
}

void TypeChecker::Visit(nodes::FuncDecl* node) {
  auto ret_type = ResolveSyntacticType(node->return_type.get());
  current_expected_return_type_ = ret_type;

  for (auto& param : node->params) {
    auto p_type = ResolveSyntacticType(param.type.get());
    param.pattern->SetResolvedType(p_type);
  }

  if (node->body != nullptr) {
    node->body->Accept(this);
  }

  current_expected_return_type_ = nullptr;
}

void TypeChecker::Visit(nodes::ModuleDecl*) {}
void TypeChecker::Visit(nodes::ImportDecl*) {}
void TypeChecker::Visit(nodes::ExportDecl* node) {
  if (node->exported_item) node->exported_item->Accept(this);
}
void TypeChecker::Visit(nodes::Attribute*) {}
void TypeChecker::Visit(nodes::Lifetime*) {}
void TypeChecker::Visit(nodes::GenericParameter*) {}
void TypeChecker::Visit(nodes::PlaceholderType*) {}
void TypeChecker::Visit(nodes::WildcardPattern* node) {
    node->SetResolvedType(last_type_);
}
void TypeChecker::Visit(nodes::BindingPattern* node) {
}

void TypeChecker::Visit(nodes::LiteralPattern* node) {
  node->SetResolvedType(last_type_);
}

void TypeChecker::Visit(nodes::RangePattern* node) {
  if (node->start != nullptr) {
    node->start->Accept(this);
  }
  if (node->end != nullptr) {
    node->end->Accept(this);
  }
  node->SetResolvedType(last_type_);
}

void TypeChecker::Visit(nodes::TuplePattern* node) {
  for (auto& element : node->elements) {
    if (element != nullptr) {
      element->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::StructPattern* node) {
  for (auto& field : node->fields) {
    if (field.pattern != nullptr) {
      field.pattern->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::ReferencePattern* node) {
  if (node->pattern != nullptr) {
    node->pattern->Accept(this);
  }
}

void TypeChecker::Visit(nodes::PrimitiveType* node) {
}

void TypeChecker::Visit(nodes::ReferenceType* node) {
  if (node->base != nullptr) {
    node->base->Accept(this);
  }
}

void TypeChecker::Visit(nodes::PointerType* node) {
  if (node->base != nullptr) {
    node->base->Accept(this);
  }
}

void TypeChecker::Visit(nodes::ArrayType* node) {
  if (node->base != nullptr) {
    node->base->Accept(this);
  }
  if (node->size_expr != nullptr) {
    node->size_expr->Accept(this);
  }
}

void TypeChecker::Visit(nodes::GenericType* node) {
  for (auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::FunctionType* node) {
  for (auto& param : node->params) {
    if (param != nullptr) {
      param->Accept(this);
    }
  }
  if (node->return_type != nullptr) {
    node->return_type->Accept(this);
  }
}

void TypeChecker::Visit(nodes::UnaryExpr* node) {
  if (node->operand != nullptr) {
    node->operand->Accept(this);
    node->resolved_type = last_type_;
  }
}

void TypeChecker::Visit(nodes::SpaceshipExpr* node) {
  node->left->Accept(this);
  auto left_type = last_type_;
  node->right->Accept(this);

  last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Int32);
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::CallExpr* node) {
  if (node->callee != nullptr) {
    node->callee->Accept(this);
  }

  for (auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::MethodCallExpr* node) {
  if (node->object == nullptr) {
    return;
  }

  // 1. Evaluate the type of the receiver object (e.g., 'obj' in 'obj.method()')
  node->object->Accept(this);
  std::shared_ptr<Type> receiver_type = last_type_;

  if (receiver_type == nullptr) {
    return;
  }

  Symbol* method_symbol = nullptr;

  if (receiver_type->GetKind() == TypeKind::Struct) {
    auto* struct_type = dynamic_cast<StructType*>(receiver_type.get());
    method_symbol = struct_type->LookupMethod(node->method_name);
  } else if (receiver_type->GetKind() == TypeKind::Pointer) {
    auto* ptr_type = dynamic_cast<PointerType*>(receiver_type.get());
    if (ptr_type->GetBaseType()->GetKind() == TypeKind::Struct) {
       auto* struct_type = static_cast<StructType*>(ptr_type->GetBaseType().get());
       method_symbol = struct_type->LookupMethod(node->method_name);
    }
  }

  if (method_symbol == nullptr) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Type '" + receiver_type->ToString() + "' has no method named '" + node->method_name + "'"
    );
    return;
  }

  node->resolved_method = method_symbol;

  auto method_type = std::dynamic_pointer_cast<FunctionType>(method_symbol->GetType());
  if (method_type != nullptr) {
    last_type_ = method_type->GetReturnType();
    node->resolved_type = last_type_;
  }

  for (const auto & argument : node->arguments) {
    if (argument != nullptr) {
      argument->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::MemberAccessExpr* node) {
  if (node->object != nullptr) {
    node->object->Accept(this);
  }
}

void TypeChecker::Visit(nodes::IndexAccessExpr* node) {
  if (node->object != nullptr) {
    node->object->Accept(this);
  }
  if (node->index != nullptr) {
    node->index->Accept(this);
  }
}

void TypeChecker::Visit(nodes::IfExpr* node) {
  node->condition->Accept(this);

  node->then_branch->Accept(this);
  auto then_type = last_type_;

  if (node->else_branch != nullptr) {
    node->else_branch->Accept(this);
    auto else_type = last_type_;

    if (not then_type->IsEqualTo(else_type.get())) {
      common::DiagnosticEngine::GetInstance().Report(
          common::DiagnosticLevel::Error, node->location, "If-else branch type mismatch");
    }
  }

  last_type_ = then_type;
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::MatchExpr* node) {
  node->value->Accept(this);
  auto match_val_type = last_type_;

  std::shared_ptr<Type> arm_type = nullptr;

  for (auto& arm : node->arms) {
    if (arm.pattern != nullptr) {
      arm.pattern->Accept(this);
    }
    if (arm.body != nullptr) {
      arm.body->Accept(this);
      if (arm_type == nullptr) {
        arm_type = last_type_;
      } else if (not arm_type->IsEqualTo(last_type_.get())) {
        common::DiagnosticEngine::GetInstance().Report(
            common::DiagnosticLevel::Error, node->location, "Match arms have inconsistent types");
      }
    }
  }

  last_type_ = arm_type;
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::LoopExpr* node) {
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::TryExpr* node) {
  if (node->expr != nullptr) {
    node->expr->Accept(this);
  }
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::NewExpr* node) {
  auto base_type = ResolveSyntacticType(node->target_type.get());

  for (auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
    }
  }

  last_type_ = std::make_shared<PointerType>(PointerType::Kind::Box, base_type);
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::LambdaExpr* node) {
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void TypeChecker::Visit(nodes::SizeofExpr* node) {
  last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::UInt64);
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::AlignofExpr* node) {
  last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::UInt64);
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::DecltypeExpr* node) {
  if (node->expr != nullptr) {
    node->expr->Accept(this);
  }
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::ReflectExpr* node) {
  last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Unit);
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::SelfExpr* node) {
  if (node->symbol != nullptr) {
    last_type_ = node->symbol->GetType();
    node->resolved_type = last_type_;
  }
}

void TypeChecker::Visit(nodes::BorrowExpr* node) {
  if (node->operand != nullptr) {
    node->operand->Accept(this);
    auto base = last_type_;
    last_type_ = std::make_shared<ReferenceType>(base, node->is_mutable);
    node->resolved_type = last_type_;
  }
}

void TypeChecker::Visit(nodes::UnitExpr* node) {
  last_type_ = std::make_shared<PrimitiveType>(PrimitiveType::Kind::Unit);
  node->resolved_type = last_type_;
}

void TypeChecker::Visit(nodes::ExprStmt* node) {
  if (node->expr != nullptr) {
    node->expr->Accept(this);
  }
}

void TypeChecker::Visit(nodes::WhileStmt* node) {
  if (node->condition != nullptr) {
    node->condition->Accept(this);
  }
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void TypeChecker::Visit(nodes::ForInStmt* node) {
  if (node->iterable != nullptr) {
    node->iterable->Accept(this);
  }
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void TypeChecker::Visit(nodes::BreakStmt* node) {
  if (node->value != nullptr) {
    node->value->Accept(this);
  }
}

void TypeChecker::Visit(nodes::ContinueStmt*) {}

void TypeChecker::Visit(nodes::DeferStmt* node) {
  if (node->body != nullptr) {
    node->body->Accept(this);
  }
}

void TypeChecker::Visit(nodes::StaticIfStmt* node) {
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

void TypeChecker::Visit(nodes::StructDecl* node) {
  for (auto& field : node->fields) {
    if (field.type != nullptr) {
      field.type->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::EnumDecl* node) {
  for (auto& variant : node->variants) {
    for (auto& type : variant.types) {
      if (type != nullptr) {
        type->Accept(this);
      }
    }
  }
}

void TypeChecker::Visit(nodes::ImplDecl* node) {
  for (auto& method : node->methods) {
    if (method != nullptr) {
      method->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::TraitDecl* node) {
  for (auto& method : node->methods) {
    if (method != nullptr) {
      method->Accept(this);
    }
  }
}

void TypeChecker::Visit(nodes::TypeAliasDecl* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }
}

void TypeChecker::Visit(nodes::RequiresClause* node) {
  if (node->condition != nullptr) {
    node->condition->Accept(this);
  }
}

void TypeChecker::Visit(nodes::WherePredicate* node) {
  if (node->target_type != nullptr) {
    node->target_type->Accept(this);
  }
}

void TypeChecker::Visit(nodes::Path*) {}
void TypeChecker::Visit(nodes::ExternDecl*) {}
void TypeChecker::Visit(nodes::Visibility*) {}

} // namespace sema