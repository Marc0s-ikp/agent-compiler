#include "ast_literal.hpp"

#include <climits>
#include <cstdlib>
#include <stdexcept>

namespace ast {

IntLiteral::IntLiteral(const std::string& text) {
    value_ = std::strtoull(text.c_str(), nullptr, 0);
    has_u_ = text.find_first_of("uU") != std::string::npos;
    text_ = text;
}

void IntLiteral::CheckRange() const {
    if (value_ > 0xFFFFFFFFULL) {
        throw std::runtime_error("integer too large: " + text_);
    }
}

TypePtr IntLiteral::GetType(const Context&) const {
    CheckRange();
    return (has_u_ || value_ > INT_MAX) ? Type::UInt() : Type::Int();
}

std::optional<long> IntLiteral::EvalConst(const Context&) const {
    CheckRange();
    return static_cast<long>(value_);
}

void IntLiteral::EmitRISC(std::ostream& os, Context&) const {
    CheckRange();
    os << "\tli a0, " << static_cast<int32_t>(static_cast<uint32_t>(value_))
       << "\n";
}

void CharLiteral::EmitRISC(std::ostream& os, Context&) const {
    os << "\tli a0, " << static_cast<int>(value_) << "\n";
}

}  // namespace ast
