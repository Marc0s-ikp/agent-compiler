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

TypePtr StringLiteral::GetType(const Context&) const {
    return Type::Array(Type::Integer(1, false), static_cast<int>(bytes_.size()) + 1);
}

std::optional<ConstValue> StringLiteral::EvalConstAddress(Context& ctx) const {
    return ConstValue{ctx.AddString(bytes_), 0};
}

// The literal is an array, so its value is its address.
std::optional<ConstValue> StringLiteral::EvalConstValue(Context& ctx) const {
    return EvalConstAddress(ctx);
}

void StringLiteral::EmitAddress(std::ostream& os, Context& ctx) const {
    os << "\tla a0, " << ctx.AddString(bytes_) << "\n";
}

void StringLiteral::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitAddress(os, ctx);
}

}  // namespace ast
