#include "ast_literal.hpp"

#include <climits>
#include <cstdlib>
#include <stdexcept>

namespace ast {

IntLiteral::IntLiteral(const std::string& text) {
    unsigned long long v = std::strtoull(text.c_str(), nullptr, 0);
    if (v > 0xFFFFFFFFULL) throw std::runtime_error("integer too large: " + text);
    bool has_u = text.find_first_of("uU") != std::string::npos;
    value_ = static_cast<uint32_t>(v);
    type_ = (has_u || v > INT_MAX) ? Type::UInt() : Type::Int();
}

TypePtr IntLiteral::GetType(const Context&) const { return type_; }

void IntLiteral::EmitRISC(std::ostream& os, Context&) const {
    os << "\tli a0, " << static_cast<int32_t>(value_) << "\n";
}

}  // namespace ast
