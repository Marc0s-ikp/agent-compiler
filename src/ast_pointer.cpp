#include "ast_pointer.hpp"

#include <stdexcept>

#include "ast_operators.hpp"
#include "emit.hpp"

namespace ast {

namespace {

// The object a pointer value points to; void and function pointees cannot
// be dereferenced.
TypePtr Pointee(const TypePtr& pointer) {
    if (!pointer->IsPointer()) {
        throw std::runtime_error("dereference of a non-pointer");
    }
    const TypePtr& target = pointer->Base();
    if (target->IsVoid() || target->IsFunction()) {
        throw std::runtime_error("dereference of an invalid pointer type");
    }
    return target;
}

// Value of the object whose address is in a0 (arrays yield the address).
void EmitLoadThroughA0(std::ostream& os, const Type& type) {
    EmitLoad(os, type, "a0", "a0", 0);
}

}  // namespace

// ---- Deref ----

TypePtr Deref::GetType(const Context& ctx) const {
    return Pointee(operand_->ValueType(ctx));
}

void Deref::EmitAddress(std::ostream& os, Context& ctx) const {
    GetType(ctx);
    operand_->EmitRISC(os, ctx);
}

void Deref::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitAddress(os, ctx);
    EmitLoadThroughA0(os, *GetType(ctx));
}

// ---- AddressOf ----

TypePtr AddressOf::GetType(const Context& ctx) const {
    return Type::Pointer(operand_->GetType(ctx));
}

void AddressOf::EmitRISC(std::ostream& os, Context& ctx) const {
    operand_->EmitAddress(os, ctx);
}

std::optional<ConstValue> AddressOf::EvalConstValue(const Context& ctx) const {
    return operand_->EvalConstAddress(ctx);
}

// ---- Index ----

Index::Index(ExprPtr base, ExprPtr index)
    : sum_(std::make_unique<BinaryArith>(BinOp::Add, std::move(base),
                                         std::move(index))) {}

TypePtr Index::GetType(const Context& ctx) const {
    return Pointee(sum_->GetType(ctx));
}

// &base[i] is the constant base + i; an element that is itself an array
// has that address as its value.
std::optional<ConstValue> Index::EvalConstAddress(const Context& ctx) const {
    return sum_->EvalConstValue(ctx);
}

std::optional<ConstValue> Index::EvalConstValue(const Context& ctx) const {
    if (!GetType(ctx)->IsArray()) return std::nullopt;
    return EvalConstAddress(ctx);
}

void Index::EmitAddress(std::ostream& os, Context& ctx) const {
    GetType(ctx);
    sum_->EmitRISC(os, ctx);
}

void Index::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitAddress(os, ctx);
    EmitLoadThroughA0(os, *GetType(ctx));
}

}  // namespace ast
