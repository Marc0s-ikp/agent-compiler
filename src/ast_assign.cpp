#include "ast_assign.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

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

// A plain assignment may also copy a whole struct or union.
TypePtr AssignTargetType(const Expr& lhs, const Context& ctx) {
    TypePtr type = lhs.GetType(ctx);
    if (!type->IsRecord()) return TargetType(lhs, ctx);
    if (!type->IsComplete()) throw std::runtime_error("assignment to an incomplete type");
    return type;
}

// Bytes ++ and -- add: 1, or the pointee size for a pointer.
int StepSize(const Type& type) {
    return type.IsPointer() ? PointeeSize(type) : 1;
}

}  // namespace

// ---- Assign ----

TypePtr Assign::GetType(const Context& ctx) const {
    return AssignTargetType(*lhs_, ctx);
}

// The right side is a struct's address; the bytes are copied to the target,
// whose address is also the value of the assignment.
void Assign::EmitRecordCopy(std::ostream& os, const Type& type,
                            int address_slot) const {
    EmitLoad(os, *AddressType(), "t1", "s0", address_slot);
    EmitCopyMem(os, "t1", "a0", type.Size(), type.Align());
    os << "\tmv a0, t1\n";
}

void Assign::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr type = GetType(ctx);
    lhs_->EmitAddress(os, ctx);
    int slot = ctx.PushTemp(AddressType());
    EmitStore(os, *AddressType(), "a0", "s0", slot);
    rhs_->EmitRISC(os, ctx);
    EmitConvert(os, *rhs_->ValueType(ctx), *type);
    if (type->IsRecord()) {
        EmitRecordCopy(os, *type, slot);
    } else {
        EmitLoad(os, *AddressType(), "t0", "s0", slot);
        EmitStore(os, *type, AccReg(*type), "t0", 0);
    }
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


// ---- --ir lowering ----

using ir::IrType;
using ir::Operand;


// The address is evaluated first, then the right side, as on the old path.
Operand Assign::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    RequireIrInteger(*type);
    Operand address = lhs_->EmitIRAddress(gen);
    Operand value = ValueAs(gen, *rhs_, *type);
    gen.Store(IRGen::ToIr(*type), value, address);
    return value;
}

// lhs op= rhs: the old value is converted to the operation's type, combined
// with the right side, and converted back for the store.
Operand CompoundAssign::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr type = GetType(ctx);
    TypePtr rhs_type = rhs_->ValueType(ctx);
    RequireIrInteger(*type);
    RequireIrInteger(*rhs_type);
    bool shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr common = shift ? Promote(type) : UsualArithmetic(type, rhs_type);
    TypePtr rhs_to = shift ? Promote(rhs_type) : common;
    IrType ir_type = IRGen::ToIr(*type);

    Operand address = lhs_->EmitIRAddress(gen);
    Operand old = gen.ConvertInt(gen.Load(ir_type, address), *type, *common);
    Operand rhs = ValueAs(gen, *rhs_, *rhs_to);
    Operand result = gen.ConvertInt(Arith(gen, op_, *common, old, rhs), *common, *type);
    gen.Store(ir_type, result, address);
    return result;
}

Operand IncDec::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    RequireIrInteger(*type);
    TypePtr wide = Promote(type);
    IrType ir_type = IRGen::ToIr(*type);

    Operand address = operand_->EmitIRAddress(gen);
    Operand old = gen.Load(ir_type, address);
    Operand step = Operand::Int(increment_ ? 1 : -1, IRGen::ToIr(*wide));
    Operand sum = Arith(gen, BinOp::Add, *wide, gen.ConvertInt(old, *type, *wide), step);
    Operand updated = gen.ConvertInt(sum, *wide, *type);
    gen.Store(ir_type, updated, address);
    return prefix_ ? updated : old;
}

}  // namespace ast
