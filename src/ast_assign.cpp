#include "ast_assign.hpp"

#include <stdexcept>

#include "emit.hpp"

namespace ast {

namespace {

// Type of a saved address.
TypePtr AddressType() { return Type::Pointer(Type::Void()); }

// An assignment target must be a scalar object: arrays are not assignable.
TypePtr TargetType(const Expr& lhs, const Context& ctx) {
    TypePtr type = lhs.GetType(ctx);
    if (type->IsArray()) throw std::runtime_error("assignment to an array");
    return RequireScalar(type);
}

// Bytes ++ and -- add: 1, or the pointee size for a pointer.
int StepSize(const Type& type) {
    return type.IsPointer() ? PointeeSize(type) : 1;
}

}  // namespace

// ---- Assign ----

TypePtr Assign::GetType(const Context& ctx) const {
    return TargetType(*lhs_, ctx);
}

void Assign::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    lhs_->EmitAddress(os, ctx);
    int slot = ctx.PushTemp(AddressType());
    EmitStore(os, *AddressType(), "a0", "s0", slot);
    rhs_->EmitRISC(os, ctx);
    EmitConvert(os, *rhs_->ValueType(ctx), *type);
    EmitLoad(os, *AddressType(), "t0", "s0", slot);
    EmitStore(os, *type, AccReg(*type), "t0", 0);
    ctx.PopTemp();
}

// ---- CompoundAssign ----

TypePtr CompoundAssign::GetType(const Context& ctx) const {
    return TargetType(*lhs_, ctx);
}

// Pointer += / -= integer: the pointer arithmetic helper does the scaling.
void CompoundAssign::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    TypePtr rhs_type = rhs_->ValueType(ctx);
    bool is_pointer = type->IsPointer();
    if (is_pointer && !(op_ == BinOp::Add || op_ == BinOp::Sub)) {
        throw std::runtime_error("invalid operands to compound assignment");
    }
    if (is_pointer && !rhs_type->IsInteger()) {
        throw std::runtime_error("pointer compound assignment needs an integer");
    }
    RequireScalar(rhs_type);
    bool is_shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr common = type;
    TypePtr rhs_to = rhs_type;
    if (!is_pointer) {
        RequireArithmetic(type);
        RequireArithmetic(rhs_type);
        // Floating operands: compute in the wider type, then convert back.
        bool real = type->IsFloating() || rhs_type->IsFloating();
        if (!real) {
            RequireInteger(type);
            RequireInteger(rhs_type);
        }
        common = is_shift ? Promote(type) : UsualArithmetic(type, rhs_type);
        rhs_to = is_shift ? Promote(rhs_type) : common;
    }

    lhs_->EmitAddress(os, ctx);
    int addr_slot = ctx.PushTemp(AddressType());
    EmitStore(os, *AddressType(), "a0", "s0", addr_slot);
    EmitLoad(os, *type, AccReg(*type), "a0", 0);
    EmitConvert(os, *type, *common);
    int value_slot = ctx.PushTemp(common);
    EmitStore(os, *common, AccReg(*common), "s0", value_slot);
    rhs_->EmitRISC(os, ctx);
    EmitConvert(os, *rhs_type, *rhs_to);
    EmitMoveToScratch(os, *rhs_to);
    EmitLoad(os, *common, AccReg(*common), "s0", value_slot);
    if (is_pointer) {
        EmitPointerAdd(os, op_ == BinOp::Sub, *type);
    } else {
        EmitBinOp(os, op_, *common);
    }
    EmitConvert(os, *common, *type);
    EmitLoad(os, *AddressType(), "t0", "s0", addr_slot);
    EmitStore(os, *type, AccReg(*type), "t0", 0);
    ctx.PopTemp();
    ctx.PopTemp();
}

// ---- IncDec ----

TypePtr IncDec::GetType(const Context& ctx) const {
    return TargetType(*operand_, ctx);
}

// ++ and -- on a float or double: the address waits in t1 and the old value
// in ft1. The new value (old +/- 1.0) is stored; fa0 keeps it for a prefix
// operator and gets the old value back for a postfix one.
void IncDec::EmitFloating(std::ostream& os, Context& ctx,
                          const Type& type) const {
    char s = type.Size() == 4 ? 's' : 'd';
    operand_->EmitAddress(os, ctx);
    os << "\tmv t1, a0\n";
    EmitLoad(os, type, "fa0", "a0", 0);
    os << "\tfmv." << s << " ft1, fa0\n";
    os << "\tli t0, 1\n\tfcvt." << s << ".w ft0, t0\n";
    os << "\t" << (increment_ ? "fadd." : "fsub.") << s << " fa0, fa0, ft0\n";
    EmitStore(os, type, "fa0", "t1", 0);
    if (!prefix_) os << "\tfmv." << s << " fa0, ft1\n";
}

void IncDec::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    if (type->IsFloating()) return EmitFloating(os, ctx, *type);
    int step = StepSize(*type) * (increment_ ? 1 : -1);
    operand_->EmitAddress(os, ctx);
    int slot = ctx.PushTemp(AddressType());
    EmitStore(os, *AddressType(), "a0", "s0", slot);
    EmitLoad(os, *type, "a0", "a0", 0);
    os << "\tmv t1, a0\n";  // old value
    EmitAddrOffset(os, "a0", "a0", step);
    EmitConvert(os, *Promote(type), *type);
    EmitLoad(os, *AddressType(), "t0", "s0", slot);
    EmitStore(os, *type, "a0", "t0", 0);
    if (!prefix_) os << "\tmv a0, t1\n";
    ctx.PopTemp();
}

}  // namespace ast
