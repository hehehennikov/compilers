#include "base.hpp"

#include "misc.hpp"

namespace parser::ast::nodes {

Base::Base(common::SourceLocation l)
    : location(l) {}

Base::Base(Base&&) noexcept = default;
Base& Base::operator=(Base&&) noexcept = default;

}  // namespace parser::ast