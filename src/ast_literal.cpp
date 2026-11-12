#include "ast_literal.hpp"
#include "ir_gen.hpp"

#include <climits>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "const_value.hpp"
#include "emit.hpp"
#include "target.hpp"

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

// strtof rounds straight to float; going through double first could round
// twice and differ in the last bit.
FloatLiteral::FloatLiteral(const std::string& text) {
    char suffix = text.back();
    is_float_ = suffix == 'f' || suffix == 'F';
    is_long_ = suffix == 'l' || suffix == 'L';
    value_ = is_float_ ? std::strtof(text.c_str(), nullptr)
                       : std::strtod(text.c_str(), nullptr);
}

void FloatLiteral::CheckSuffix() const {
    if (is_long_) throw std::runtime_error("long double is not supported");
}

TypePtr FloatLiteral::GetType(const Context&) const {
    CheckSuffix();
    return is_float_ ? Type::Float() : Type::Double();
}

std::optional<ConstValue> FloatLiteral::EvalConstValue(Context&) const {
    CheckSuffix();
    ConstValue v;
    v.is_real = true;
    v.real = value_;
    return v;
}

void FloatLiteral::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    os << "\tla t0, " << ctx.AddFloat(FloatBits(value_, type->Size()), type->Size()) << "\n";
    EmitLoad(os, *type, "fa0", "t0", 0);
}

void CharLiteral::EmitRISC(std::ostream& os, Context&) const {
    os << "\tli a0, " << static_cast<int>(value_) << "\n";
}

TypePtr StringLiteral::GetType(const Context&) const {
    return Type::Array(Type::Integer(Target().char_size, Target().char_is_signed), static_cast<int>(bytes_.size()) + 1);
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


// ---- --ir lowering ----

using ir::Operand;

Operand IntLiteral::EmitValue(IRGen& gen) const {
    CheckRange();
    return Operand::Int(*EvalConst(gen.Ctx()), IRGen::ToIr(*GetType(gen.Ctx())));
}

// A string literal is an array in read-only data, so its value is its address.
Operand StringLiteral::EmitIRAddress(IRGen& gen) const {
    return gen.AddressOf(Operand::Symbol(gen.Ctx().AddString(bytes_)));
}

Operand StringLiteral::EmitValue(IRGen& gen) const { return EmitIRAddress(gen); }

// The constant lives in the float pool (a read-only data entry); the value is
// loaded from it.
Operand FloatLiteral::EmitValue(IRGen& gen) const {
    CheckSuffix();
    return gen.FloatConstant(value_, IRGen::ToIr(*GetType(gen.Ctx())));
}

Operand CharLiteral::EmitValue(IRGen&) const {
    return Operand::Int(value_, IRGen::ToIr(*Type::Int()));
}

}  // namespace ast
