#include "ast_assign.hpp"

#include "emit.hpp"

namespace ast {

namespace {
// Type of a saved address. Becomes a pointer type with feature 4.
TypePtr AddressType() { return Type::UInt(); }
}  // namespace

// ---- Assign ----

TypePtr Assign::GetType(const Context& ctx) const {
    return RequireInteger(lhs_->GetType(ctx));
}

void Assign::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    lhs_->EmitAddress(os, ctx);
    int slot = ctx.PushTemp(AddressType());
    EmitStore(os, *AddressType(), "a0", "s0", slot);
    rhs_->EmitRISC(os, ctx);
    EmitConvert(os, *RequireInteger(rhs_->GetType(ctx)), *type);
    EmitLoad(os, *AddressType(), "t0", "s0", slot);
    EmitStore(os, *type, "a0", "t0", 0);
    ctx.PopTemp();
}

// ---- CompoundAssign ----

TypePtr CompoundAssign::GetType(const Context& ctx) const {
    return RequireInteger(lhs_->GetType(ctx));
}

void CompoundAssign::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    TypePtr rhs_type = RequireInteger(rhs_->GetType(ctx));
    bool is_shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr common = is_shift ? Promote(type) : UsualArithmetic(type, rhs_type);
    TypePtr rhs_to = is_shift ? Promote(rhs_type) : common;

    lhs_->EmitAddress(os, ctx);
    int addr_slot = ctx.PushTemp(AddressType());
    EmitStore(os, *AddressType(), "a0", "s0", addr_slot);
    EmitLoad(os, *type, "a0", "a0", 0);
    EmitConvert(os, *type, *common);
    int value_slot = ctx.PushTemp(common);
    EmitStore(os, *common, "a0", "s0", value_slot);
    rhs_->EmitRISC(os, ctx);
    EmitConvert(os, *rhs_type, *rhs_to);
    os << "\tmv t0, a0\n";
    EmitLoad(os, *common, "a0", "s0", value_slot);
    EmitBinOp(os, op_, *common);
    EmitConvert(os, *common, *type);
    EmitLoad(os, *AddressType(), "t0", "s0", addr_slot);
    EmitStore(os, *type, "a0", "t0", 0);
    ctx.PopTemp();
    ctx.PopTemp();
}

// ---- IncDec ----

TypePtr IncDec::GetType(const Context& ctx) const {
    return RequireInteger(operand_->GetType(ctx));
}

void IncDec::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    operand_->EmitAddress(os, ctx);
    int slot = ctx.PushTemp(AddressType());
    EmitStore(os, *AddressType(), "a0", "s0", slot);
    EmitLoad(os, *type, "a0", "a0", 0);
    os << "\tmv t1, a0\n";  // old value
    os << "\taddi a0, a0, " << (increment_ ? 1 : -1) << "\n";
    EmitConvert(os, *Promote(type), *type);
    EmitLoad(os, *AddressType(), "t0", "s0", slot);
    EmitStore(os, *type, "a0", "t0", 0);
    if (!prefix_) os << "\tmv a0, t1\n";
    ctx.PopTemp();
}

}  // namespace ast
