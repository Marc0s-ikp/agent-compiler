#include "ast_pointer.hpp"

#include <stdexcept>

#include "ast_lower.hpp"
#include "ast_operators.hpp"

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

}  // namespace

// ---- Deref ----

TypePtr Deref::GetType(const Context& ctx) const {
    return Pointee(operand_->ValueType(ctx));
}

// ---- AddressOf ----

TypePtr AddressOf::GetType(const Context& ctx) const {
    return Type::Pointer(operand_->GetType(ctx));
}

std::optional<ConstValue> AddressOf::EvalConstValue(Context& ctx) const {
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
std::optional<ConstValue> Index::EvalConstAddress(Context& ctx) const {
    return sum_->EvalConstValue(ctx);
}

std::optional<ConstValue> Index::EvalConstValue(Context& ctx) const {
    if (!GetType(ctx)->IsArray()) return std::nullopt;
    return EvalConstAddress(ctx);
}

// *p is the object at the pointer's value; a[i] is *(a + i), so its address is
// the ptradd the sum lowers to. An array object is its own address (D10).
using ir::Operand;

Operand Deref::EmitIRAddress(IRGen& gen) const {
    GetType(gen.Ctx());  // rejects a pointer to void or to a function
    return operand_->EmitValue(gen);
}

Operand Deref::EmitValue(IRGen& gen) const {
    return LoadObject(gen, EmitIRAddress(gen), *GetType(gen.Ctx()));
}

Operand AddressOf::EmitValue(IRGen& gen) const {
    return operand_->EmitIRAddress(gen);
}

Operand Index::EmitIRAddress(IRGen& gen) const {
    // Called only for its checks (a non-pointer base throws here); the address
    // is the sum, a ptradd.
    GetType(gen.Ctx());
    return sum_->EmitValue(gen);
}

Operand Index::EmitValue(IRGen& gen) const {
    return LoadObject(gen, EmitIRAddress(gen), *GetType(gen.Ctx()));
}

}  // namespace ast
