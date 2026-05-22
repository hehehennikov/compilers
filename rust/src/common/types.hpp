#pragma once

#include <vector>

struct Type {
  enum class Kind {
    Int,
    Float,
    Bool,
    Function,
    Lazy,
    Placeholder
  };

  Kind kind;
  std::vector<Type> params; // для шаблонных классов или параметров функций
  bool is_mutable;
};