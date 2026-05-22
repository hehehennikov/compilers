#pragma once

#include <string>
#include <vector>
#include <memory>

#include <lexer/token.hpp>
#include <magic_enum/magic_enum.hpp>


namespace sema {

class Symbol;

enum class TypeKind {
  Primitive,
  Pointer,
  Reference,
  Function,
  Struct,
  Enum,
  Placeholder
};

class Type {
 public:
  virtual ~Type() = default;

 public:
  [[nodiscard]]
  virtual TypeKind GetKind() const = 0;

  virtual bool IsEqualTo(const Type* other) const = 0;

  [[nodiscard]]
  virtual std::string ToString() const = 0;

  [[nodiscard]]
  virtual size_t GetSizeInBytes() const = 0;

  [[nodiscard]]
  virtual bool IsCopy() const {
    return false;
  }
};

class PrimitiveType : public Type {
 public:
  enum class Kind {
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    Bool,
    Unit,
    String,
    ISize,
    USize
  };

 public:
  explicit PrimitiveType(Kind kind)
      : kind_(kind) {}

  explicit PrimitiveType(lexer::TokenType token_kind) {
    switch (token_kind) {
      case lexer::TokenType::Int8:    kind_ = Kind::Int8; break;
      case lexer::TokenType::Int16:   kind_ = Kind::Int16; break;
      case lexer::TokenType::Int32:   kind_ = Kind::Int32; break;
      case lexer::TokenType::Int64:   kind_ = Kind::Int64; break;
      case lexer::TokenType::UInt8:   kind_ = Kind::UInt8; break;
      case lexer::TokenType::UInt16:  kind_ = Kind::UInt16; break;
      case lexer::TokenType::UInt32:  kind_ = Kind::UInt32; break;
      case lexer::TokenType::UInt64:  kind_ = Kind::UInt64; break;
      case lexer::TokenType::Float32: kind_ = Kind::Float32; break;
      case lexer::TokenType::Float64: kind_ = Kind::Float64; break;
      case lexer::TokenType::Bool:    kind_ = Kind::Bool; break;
      case lexer::TokenType::String:  kind_ = Kind::String; break;
      case lexer::TokenType::ISize:   kind_ = Kind::ISize; break;
      case lexer::TokenType::USize:   kind_ = Kind::USize; break;
      default:                        kind_ = Kind::Unit; break;
    }
  }


 public:
  [[nodiscard]]
  TypeKind GetKind() const override {
    return TypeKind::Primitive;
  }

  [[nodiscard]]
  bool IsEqualTo(const Type* other) const override {
    if (other->GetKind() != TypeKind::Primitive) {
      return false;
    }

    const auto& other_prim = dynamic_cast<const PrimitiveType*>(other);
    return kind_ == other_prim->kind_;
  }

  [[nodiscard]]
  auto GetSizeInBytes() const -> size_t override {
    switch (kind_) {
      case Kind::Int8:    return 1;
      case Kind::Int16:   return 2;
      case Kind::Int32:   return 4;
      case Kind::Int64:   return 8;
      case Kind::UInt8:   return 1;
      case Kind::UInt16:  return 2;
      case Kind::UInt32:  return 4;
      case Kind::UInt64:  return 8;
      case Kind::Float32: return 4;
      case Kind::Float64: return 8;
      case Kind::Bool:    return 1;
      case Kind::Unit:    return 0;
      case Kind::ISize:   return sizeof(std::size_t);
      case Kind::USize:   return sizeof(std::size_t);
      case Kind::String:  return sizeof(std::string_view);
    }

    return 0;
  }

  [[nodiscard]]
  std::string ToString() const override {
    return std::string(magic_enum::enum_name(kind_));
  }

  [[nodiscard]]
  bool IsCopy() const override {
    return true;
  }

 private:
  Kind kind_;
};

class ReferenceType : public Type {
 public:
  ReferenceType(std::shared_ptr<Type> base,
                bool is_mutable,
                std::string lifetime = "")
      : base_(std::move(base)),
        is_mutable_(is_mutable),
        lifetime_(std::move(lifetime)) {}

 public:
  [[nodiscard]]
  TypeKind GetKind() const override {
    return TypeKind::Reference;
  }

  bool IsEqualTo(const Type* other) const override {
    if (other->GetKind() != TypeKind::Reference) {
      return false;
    }

    auto* o = dynamic_cast<const ReferenceType*>(other);
    return is_mutable_ == o->is_mutable_ && base_->IsEqualTo(o->base_.get());
  }

  [[nodiscard]]
  std::string ToString() const override {
    std::string res = "&";
    if (not lifetime_.empty()) {
      res += "'" + lifetime_ + " ";
    }
    if (is_mutable_) {
      res += "mut ";
    }
    return res + base_->ToString();
  }

  [[nodiscard]]
  std::size_t GetSizeInBytes() const override {
    return sizeof(void*);
  }

  [[nodiscard]]
  bool IsCopy() const override {
    return !is_mutable_;
  }

 private:
  std::shared_ptr<Type> base_;
  bool is_mutable_;
  std::string lifetime_;
};

class PointerType : public Type {
 public:
  enum class Kind {
    RawConst,
    RawMut,
    Box,
    Unique
  };

 public:
  PointerType(Kind kind, std::shared_ptr<Type> base)
      : kind_(kind), base_(std::move(base)) {}

 public:
  [[nodiscard]]
  TypeKind GetKind() const override {
    return TypeKind::Pointer;
  }

  bool IsEqualTo(const Type* other) const override {
    if (other->GetKind() != TypeKind::Pointer) {
      return false;
    }

    auto* o = dynamic_cast<const PointerType*>(other);
    return kind_ == o->kind_ && base_->IsEqualTo(o->base_.get());
  }

  [[nodiscard]]
  std::string ToString() const override {
    if (kind_ == Kind::Box) {
      return "Box<" + base_->ToString() + ">";
    }
    return "*pointer";
  }

  [[nodiscard]]
  std::size_t GetSizeInBytes() const override {
    return sizeof(void*);
  }

  [[nodiscard]]
  std::shared_ptr<Type> GetBaseType() const {
    return base_;
  }

  [[nodiscard]]
  bool IsCopy() const override {
    return kind_ != Kind::Box;
  }

 private:
  Kind kind_;
  std::shared_ptr<Type> base_;
};

struct StructField {
  std::string name;
  std::shared_ptr<Type> type;
};

class StructType : public Type {
 public:
  explicit StructType(std::string name, std::vector<StructField> fields)
      : name_(std::move(name)), fields_(std::move(fields)) {}

 public:
  [[nodiscard]]
  TypeKind GetKind() const override {
    return TypeKind::Struct;
  }

  bool IsEqualTo(const Type* other) const override {
    if (other->GetKind() != TypeKind::Struct) {
      return false;
    }

    return name_ == dynamic_cast<const StructType*>(other)->name_;
  }

  [[nodiscard]]
  std::string ToString() const override {
    return name_;
  }

  [[nodiscard]]
  std::size_t GetSizeInBytes() const override {
    size_t total = 0;
    for (const auto& field : fields_) {
      total += field.type->GetSizeInBytes();
    }

    return total;
  }

  void AddMethod(const std::string& name, Symbol* method_symbol) {
    methods_[name] = method_symbol;
  }

  [[nodiscard]]
  Symbol* LookupMethod(const std::string& name) const {
    if (methods_.contains(name)) {
      return methods_.at(name);
    }

    return nullptr;
  }

 private:
  std::string name_;
  std::vector<StructField> fields_;
  std::unordered_map<std::string, Symbol*> methods_;
};

class FunctionType : public Type {
 public:
  FunctionType(std::vector<std::shared_ptr<Type>> params, std::shared_ptr<Type> ret)
      : params_(std::move(params)), return_type_(std::move(ret)) {}

 public:
  [[nodiscard]]
  TypeKind GetKind() const override {
    return TypeKind::Function;
  }

  bool IsEqualTo(const Type* other) const override {
    if (other->GetKind() != TypeKind::Function) {
      return false;
    }

    auto* o = dynamic_cast<const FunctionType*>(other);
    if (params_.size() != o->params_.size()) {
      return false;
    }

    for (size_t i = 0; i < params_.size(); ++i) {
      if (!params_[i]->IsEqualTo(o->params_[i].get())) {
        return false;
      }
    }

    return return_type_->IsEqualTo(o->return_type_.get());
  }

  [[nodiscard]]
  std::shared_ptr<Type> GetReturnType() const {
    if (return_type_ == nullptr) {
      return std::make_shared<PrimitiveType>(lexer::TokenType::Unit);
    }
    return return_type_;
  }

  [[nodiscard]]
  std::shared_ptr<Type> GetParamType(std::size_t index) const {
    if (index < params_.size()) {
      return params_[index];
    }
    return nullptr;
  }


  [[nodiscard]]
  std::string ToString() const override {
    std::string res = "fn(";
    for (auto p : params_) {
      res += p->ToString();
    }
    res += ')';
    return res;
  }

  [[nodiscard]]
  std::size_t GetSizeInBytes() const override {
    return sizeof(void*);
  }

 private:
  std::vector<std::shared_ptr<Type>> params_;
  std::shared_ptr<Type> return_type_;
};

} // namespace sema