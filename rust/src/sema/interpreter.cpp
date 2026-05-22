#include "sema/interpreter.hpp"

#include <iostream>
#include <ranges>
#include <parser/ast/nodes/all.hpp>
#include <common/diagnostic.hpp>

namespace sema {

using namespace parser::ast;

struct BreakSignal {
  Value value;
};

struct ContinueSignal {};

struct ReturnSignal {
  Value value;
};

static RuntimeValueData ConvertTokenToRuntime(const lexer::TokenData& data) {
  return std::visit([](auto&& arg) -> RuntimeValueData {
    return arg;
  }, data);
}

static bool AreValuesEqual(const RuntimeValueData& lhs, const RuntimeValueData& rhs) {
  return std::visit([]<typename Lhs, typename Rhs>(Lhs&& l, Rhs&& r) -> bool {
    using TL = std::decay_t<Lhs>;
    using TR = std::decay_t<Rhs>;

    if constexpr (std::is_same_v<TL, TR>) {
      if constexpr (std::is_same_v<TL, std::monostate>) {
        return true;
      } else if constexpr (std::is_same_v<TL, std::vector<Value>>) {
        if (l.size() != r.size()) return false;
        for (size_t i = 0; i < l.size(); ++i) {
          if (!AreValuesEqual(l[i].data, r[i].data)) return false;
        }
        return true;
      } else if constexpr (requires { l == r; }) {
        return l == r;
      }
    }
    return false;
  }, lhs, rhs);
}

Interpreter::Interpreter() {
  call_stack_.emplace_back();
  last_value_ = {std::monostate{}};
}

void Interpreter::Run(const std::vector<std::unique_ptr<nodes::Base>>& program) {
  try {
    for (const auto& node : program) {
      if (node != nullptr) {
        node->Accept(this);
      }
    }
  } catch (const ReturnSignal& s) {
    last_value_ = s.value;
  } catch (...) {
    std::unreachable();
  }
}

void Interpreter::PushFrame() {
  call_stack_.emplace_back();
}

void Interpreter::PopFrame() {
  if (call_stack_.size() > 1) {
    call_stack_.pop_back();
  }
}

Value& Interpreter::GetValue(Symbol* sym) {
  if (CurrentFrame().contains(sym)) {
    return CurrentFrame()[sym];
  }

  // Ищем в глобальном фрейме
  if (call_stack_[0].contains(sym)) {
    return call_stack_[0][sym];
  }

  static Value error_val = {std::monostate{}};
  return error_val;
}

bool Interpreter::MatchPattern(nodes::Pattern* pat, const Value& val) {
  if (pat == nullptr) {
    return false;
  }

  if (dynamic_cast<nodes::WildcardPattern*>(pat) != nullptr) {
    return true;
  }

  if (auto* bind = dynamic_cast<nodes::BindingPattern*>(pat)) {
    if (bind->symbol != nullptr) {
      CurrentFrame()[bind->symbol] = val;
    }
    return true;
  }

  if (auto* lit_pat = dynamic_cast<nodes::LiteralPattern*>(pat)) {
    return CompareWithToken(val.data, lit_pat->value);
  }

  if (auto* tuple_pat = dynamic_cast<nodes::TuplePattern*>(pat)) {
    if (!val.Is<std::vector<Value>>()) {
      return false;
    }

    const auto& elements = val.As<std::vector<Value>>();
    if (elements.size() != tuple_pat->elements.size()) {
      return false;
    }

    for (size_t i = 0; i < elements.size(); ++i) {
      if (!MatchPattern(tuple_pat->elements[i].get(), elements[i])) {
        return false;
      }
    }
    return true;
  }

  return false;
}

void Interpreter::Visit(nodes::LiteralExpr* node) {
  last_value_.data = ConvertTokenToRuntime(node->value);
}

void Interpreter::Visit(nodes::IdentExpr* node) {
  last_value_ = GetValue(node->symbol);
}

void Interpreter::Visit(nodes::BinaryExpr* node) {
  node->left->Accept(this);
  Value lhs = last_value_;
  node->right->Accept(this);
  Value rhs = last_value_;

  if (node->op == lexer::TokenType::EqEq) {
    last_value_.data = AreValuesEqual(lhs.data, rhs.data);
    return;
  }

  auto perform = [&]<typename T>(T l, T r) -> RuntimeValueData {
    if constexpr (std::is_arithmetic_v<T>) {
      switch (node->op) {
        case lexer::TokenType::Plus: return l + r;
        case lexer::TokenType::Minus: return l - r;
        case lexer::TokenType::Star: return l * r;
        case lexer::TokenType::Slash: return (r != 0) ? (l / r) : l;
        case lexer::TokenType::Less: return l < r;
        case lexer::TokenType::Greater: return l > r;
        default: return std::monostate{};
      }
    }
    if constexpr (std::is_same_v<T, std::string>) {
      if (node->op == lexer::TokenType::Plus) return l + r;
    }
    return std::monostate{};
  };;

  std::visit([&]<typename Lhs, typename Rhs>(Lhs&& l, Rhs&& r) {
    using TL = std::decay_t<Lhs>;
    using TR = std::decay_t<Rhs>;

    if constexpr (std::is_same_v<TL, TR>) {
      last_value_.data = perform(l, r);
    } else {
      last_value_.data = std::monostate{};
    }
  }, lhs.data, rhs.data);
}

void Interpreter::Visit(nodes::UnaryExpr* node) {
  node->operand->Accept(this);

  if (node->op == lexer::TokenType::Exclamation && last_value_.Is<bool>()) {
    last_value_.data = !last_value_.As<bool>();
  } else if (node->op == lexer::TokenType::Minus) {
    if (last_value_.Is<int32_t>()) {
      last_value_.data = -last_value_.As<int32_t>();
    } else if (last_value_.Is<double>()) {
      last_value_.data = -last_value_.As<double>();
    }
  }
}

void Interpreter::Visit(nodes::BlockExpr* node) {
  deferred_stack_.emplace_back();


  try {
    for (const auto& stmt : node->statements) {
      if (stmt != nullptr) {
        stmt->Accept(this);
      }
    }

    if (node->final_expression != nullptr) {
      node->final_expression->Accept(this);
    }
  } catch (...) {
    auto current_defers = std::move(deferred_stack_.back());
    deferred_stack_.pop_back();


    for (auto & current_defer : std::ranges::reverse_view(current_defers)) {
      current_defer->Accept(this);
    }

    throw;
  }

  auto current_defers = std::move(deferred_stack_.back());
  deferred_stack_.pop_back();

  Value result_backup = last_value_;

  for (auto & current_defer : std::ranges::reverse_view(current_defers)) {
    current_defer->Accept(this);
  }

  // Restore the result of the block
  last_value_ = result_backup;
}

void Interpreter::Visit(nodes::IfExpr* node) {
  node->condition->Accept(this);

  if (last_value_.Is<bool>() && last_value_.As<bool>()) {
    node->then_branch->Accept(this);
  } else if (node->else_branch != nullptr) {
    node->else_branch->Accept(this);
  }
}

void Interpreter::Visit(nodes::WhileStmt* node) {
  while (true) {
    node->condition->Accept(this);

    if (!last_value_.Is<bool>() || !last_value_.As<bool>()) {
      break;
    }

    try {
      node->body->Accept(this);
    } catch (const BreakSignal&) {
      break;
    } catch (const ContinueSignal&) {
      continue;
    }
  }
}

void Interpreter::Visit(nodes::LoopExpr* node) {
  while (true) {
    try {
      node->body->Accept(this);
    } catch (const BreakSignal& s) {
      last_value_ = s.value;
      break;
    } catch (const ContinueSignal&) {
      continue;
    }
  }
}

void Interpreter::Visit(nodes::BreakStmt* node) {
  Value val = {std::monostate{}};
  if (node->value != nullptr) {
    node->value->Accept(this);
    val = last_value_;
  }
  throw BreakSignal{val};
}

void Interpreter::Visit(nodes::ContinueStmt*) {
  throw ContinueSignal{};
}

void Interpreter::Visit(nodes::ReturnStmt* node) {
  Value val = {std::monostate{}};
  if (node->value != nullptr) {
    node->value->Accept(this);
    val = last_value_;
  }
  throw ReturnSignal{val};
}

void Interpreter::Visit(nodes::CallExpr* node) {
  if (auto* ident = dynamic_cast<nodes::IdentExpr*>(node->callee.get())) {
    if (ident->name == "print") {
      for (const auto& arg : node->arguments) {
        arg->Accept(this);
        std::visit([]<typename U>(U&& v) {
          using T = std::decay_t<U>;
          if constexpr (requires { std::cout << v; }) {
            std::cout << v;
          } else {
            std::cout << "[complex value]";
          }
        }, last_value_.data);
      }
      std::cout << '\n';
      last_value_.data = std::monostate{};
      return;
    }
  }

  node->callee->Accept(this);
  if (auto* callee_ident = dynamic_cast<nodes::IdentExpr*>(node->callee.get())) {
    auto* func = dynamic_cast<nodes::FuncDecl*>(callee_ident->symbol->GetDeclNode());

    if (func != nullptr && func->body != nullptr) {
      std::vector<Value> args;
      for (auto& arg : node->arguments) {
        arg->Accept(this);
        args.push_back(last_value_);
      }

      PushFrame();
      for (size_t i = 0; i < func->params.size(); ++i) {
        auto* param_bind = dynamic_cast<nodes::BindingPattern*>(func->params[i].pattern.get());
        CurrentFrame()[param_bind->symbol] = args[i];
      }

      try {
        func->body->Accept(this);
      } catch (const ReturnSignal& s) {
        last_value_ = s.value;
      }
      PopFrame();
    }
  }
}

void Interpreter::Visit(nodes::NewExpr* node) {
  auto obj = std::make_shared<Object>();
  if (auto* gen_type = dynamic_cast<nodes::GenericType*>(node->target_type.get())) {
    last_value_.data = obj;
  }
}

void Interpreter::Visit(nodes::MemberAccessExpr* node) {
  node->object->Accept(this);
  if (last_value_.Is<std::shared_ptr<Object>>()) {
    auto obj = last_value_.As<std::shared_ptr<Object>>();
    if (obj->fields.contains(node->member_name)) {
      last_value_ = obj->fields.at(node->member_name);
    }
  }
}

void Interpreter::Visit(nodes::LetStmt* node) {
  if (node->initializer != nullptr) {
    node->initializer->Accept(this);
  } else {
    last_value_.data = std::monostate{};
  }

  if (node->pattern != nullptr) {
    MatchPattern(node->pattern.get(), last_value_);
  }
}

void Interpreter::Visit(nodes::BindingPattern* node) {
  if (node->symbol != nullptr) {
    CurrentFrame()[node->symbol] = last_value_;
  }
}

void Interpreter::Visit(nodes::TuplePattern* node) {
}

void Interpreter::Visit(nodes::ExprStmt* node) { if (node->expr) node->expr->Accept(this); }
void Interpreter::Visit(nodes::AssignStmt* node) {
  node->rhs->Accept(this);
  Value new_val = last_value_;

  if (auto* ident = dynamic_cast<nodes::IdentExpr*>(node->lhs.get())) {
    if (ident->symbol != nullptr) {
      GetValue(ident->symbol) = new_val;
    }
  }
}
void Interpreter::Visit(nodes::StaticIfStmt* node) {
  node->condition->Accept(this);
  if (last_value_.As<bool>()) {
    node->then_branch->Accept(this);
  } else if (node->else_branch) {
    node->else_branch->Accept(this);
  }
}

void Interpreter::Visit(nodes::ModuleDecl*) {}
void Interpreter::Visit(nodes::ImportDecl*) {}
void Interpreter::Visit(nodes::ExportDecl* n) { if (n->exported_item) n->exported_item->Accept(this); }
void Interpreter::Visit(nodes::ExternDecl*) {}
void Interpreter::Visit(nodes::Visibility*) {}
void Interpreter::Visit(nodes::Attribute*) {}
void Interpreter::Visit(nodes::Path*) {}
void Interpreter::Visit(nodes::Lifetime*) {}
void Interpreter::Visit(nodes::GenericParameter*) {}
void Interpreter::Visit(nodes::RequiresClause*) {}
void Interpreter::Visit(nodes::WherePredicate*) {}
void Interpreter::Visit(nodes::SelfExpr* n) { last_value_ = GetValue(n->symbol); }
void Interpreter::Visit(nodes::BorrowExpr* n) { n->operand->Accept(this); }
void Interpreter::Visit(nodes::SpaceshipExpr*) {}
void Interpreter::Visit(nodes::CastExpr* n) { n->expr->Accept(this); }

void Interpreter::Visit(nodes::MatchExpr* node) {
  // calculate the value to match against
  node->value->Accept(this);
  Value target_val = last_value_;

  for (auto& arm : node->arms) {

    // try to match the pattern
    if (MatchPattern(arm.pattern.get(), target_val)) {
      // check 'if' guard if it exists (e.g., match x { n if n > 0 => ... })
      if (arm.guard != nullptr) {
        arm.guard->Accept(this);
        if (not last_value_.Is<bool>() || !last_value_.As<bool>()) {
          ///TODO: undo bindings.
          continue;
        }
      }

      arm.body->Accept(this);
      return;
    }
  }

  last_value_.data = std::monostate{};
}

void Interpreter::Visit(nodes::TryExpr*) {}
void Interpreter::Visit(nodes::LambdaExpr*) {}
void Interpreter::Visit(nodes::SizeofExpr*) { last_value_.data = (int32_t)8; }
void Interpreter::Visit(nodes::AlignofExpr*) { last_value_.data = (int32_t)8; }
void Interpreter::Visit(nodes::DecltypeExpr*) {}
void Interpreter::Visit(nodes::ReflectExpr*) {}
void Interpreter::Visit(nodes::ForInStmt*) {}
void Interpreter::Visit(nodes::FuncDecl*) {}
void Interpreter::Visit(nodes::StructDecl*) {}
void Interpreter::Visit(nodes::EnumDecl*) {}
void Interpreter::Visit(nodes::ImplDecl*) {}
void Interpreter::Visit(nodes::TraitDecl*) {}
void Interpreter::Visit(nodes::TypeAliasDecl*) {}
void Interpreter::Visit(nodes::PrimitiveType*) {}
void Interpreter::Visit(nodes::ReferenceType*) {}
void Interpreter::Visit(nodes::PointerType*) {}
void Interpreter::Visit(nodes::ArrayType*) {}
void Interpreter::Visit(nodes::GenericType*) {}
void Interpreter::Visit(nodes::FunctionType*) {}
void Interpreter::Visit(nodes::PlaceholderType*) {}
void Interpreter::Visit(nodes::WildcardPattern*) {}
void Interpreter::Visit(nodes::LiteralPattern*) {}
void Interpreter::Visit(nodes::RangePattern*) {}
void Interpreter::Visit(nodes::StructPattern*) {}
void Interpreter::Visit(nodes::ReferencePattern*) {}
void Interpreter::Visit(nodes::UnitExpr* ) {
  last_value_.data = std::monostate{};
}
void Interpreter::Visit(nodes::IndexAccessExpr* node) {
  node->object->Accept(this);
  Value container = last_value_;

  node->index->Accept(this);

  std::size_t idx = 0;
  if (last_value_.Is<std::int32_t>()) {
    idx = static_cast<std::size_t>(last_value_.As<std::int32_t>());
  } else if (last_value_.Is<std::uint64_t>()) {
    idx = last_value_.As<std::uint64_t>();
  }

  if (container.Is<std::vector<Value>>()) {
    const auto& vec = container.As<std::vector<Value>>();
    if (idx < vec.size()) {
      last_value_ = vec.at(idx);
    } else {
      std::cerr << "Runtime Error: Index out of bounds" << '\n';
      std::exit(1);
    }
  }
}

void Interpreter::Visit(nodes::MethodCallExpr* node) {
  if (node->object == nullptr) {
    return;
  }
  node->object->Accept(this);
  Value receiver_value = last_value_;

  std::vector<Value> evaluated_args;
  for (const auto& arg : node->arguments) {
    if (arg != nullptr) {
      arg->Accept(this);
      evaluated_args.push_back(last_value_);
    }
  }

  if (node->resolved_method == nullptr) {
    common::DiagnosticEngine::GetInstance().Report(
        common::DiagnosticLevel::Error,
        node->location,
        "Runtime Error: Method '" + node->method_name + "' was not resolved during static analysis."
    );
    return;
  }

  auto* func_decl = static_cast<nodes::FuncDecl*>(node->resolved_method->GetDeclNode());

  PushFrame();

  if (not func_decl->params.empty()) {
    auto* self_param_pattern = func_decl->params[0].pattern.get();

    if (self_param_pattern != nullptr) {
      MatchPattern(self_param_pattern, receiver_value);
    }
  }

  for (std::size_t i = 0; i < evaluated_args.size(); ++i) {
    size_t param_index = i + 1;

    if (param_index < func_decl->params.size()) {
      auto* param_pattern = func_decl->params[param_index].pattern.get();

      if (param_pattern != nullptr) {
        MatchPattern(param_pattern, evaluated_args[i]);
      }
    }
  }

  try {
    if (func_decl->body != nullptr) {
      func_decl->body->Accept(this);
    }
  } catch (const ReturnSignal& signal) {
    last_value_ = signal.value;
  }

  PopFrame();
}

void Interpreter::Visit(nodes::DeferStmt* node) {
  if (!deferred_stack_.empty()) {
    deferred_stack_.back().push_back(node->body.get());
  }
}

} // namespace sema