#include "ast_assign.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

namespace ast {

namespace {

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

}  // namespace

// ---- Assign ----

TypePtr Assign::GetType(const Context& ctx) const {
    return AssignTargetType(*lhs_, ctx);
}

// ---- CompoundAssign ----

TypePtr CompoundAssign::GetType(const Context& ctx) const {
    return TargetType(*lhs_, ctx);
}

// ---- IncDec ----

TypePtr IncDec::GetType(const Context& ctx) const {
    return TargetType(*operand_, ctx);
}

using ir::IrType;
using ir::Operand;

// The address is evaluated first, then the right side.
Operand Assign::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    if (type->IsRecord()) return EmitRecordCopy(gen, *type);
    RequireIrScalar(*type);
    Operand address = lhs_->EmitIRAddress(gen);
    Operand value = ValueAs(gen, *rhs_, *type);
    gen.Store(IRGen::ToIr(*type), value, address);
    return value;
}

// A struct or union is copied byte for byte; the value of the assignment is
// the target's address.
Operand Assign::EmitRecordCopy(IRGen& gen, const Type& type) const {
    Operand address = lhs_->EmitIRAddress(gen);
    Operand source = ValueAs(gen, *rhs_, type);
    gen.MemCpy(address, source, type.Size(), type.Align());
    return address;
}

// lhs op= rhs: the old value is converted to the operation's type, combined
// with the right side, and converted back for the store. A pointer takes only
// += and -= with an integer.
Operand CompoundAssign::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr type = GetType(ctx);
    TypePtr rhs_type = rhs_->ValueType(ctx);
    IrType ir_type = IRGen::ToIr(*type);
    if (type->IsPointer()) return EmitPointerValue(gen, *type, *rhs_type);
    if (!type->IsArithmetic() || !rhs_type->IsArithmetic()) {
        throw std::runtime_error("invalid operands to binary operator");
    }
    bool shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr common = shift ? Promote(type) : UsualArithmetic(type, rhs_type);
    TypePtr rhs_to = shift ? Promote(rhs_type) : common;

    Operand address = lhs_->EmitIRAddress(gen);
    Operand old = ConvertScalar(gen, gen.Load(ir_type, address), *type, *common);
    Operand rhs = ValueAs(gen, *rhs_, *rhs_to);
    Operand result = ConvertScalar(gen, Arith(gen, op_, *common, old, rhs), *common, *type);
    gen.Store(ir_type, result, address);
    return result;
}

Operand CompoundAssign::EmitPointerValue(IRGen& gen, const Type& type, const Type& rhs_type) const {
    if (op_ != BinOp::Add && op_ != BinOp::Sub) throw std::runtime_error("invalid operands to binary operator");
    if (!rhs_type.IsInteger()) throw std::runtime_error("invalid operands to binary operator");
    Operand address = lhs_->EmitIRAddress(gen);
    Operand old = gen.Load(ir::IrType::Ptr, address);
    Operand index = rhs_->EmitValue(gen);
    Operand result = PointerOffset(gen, old, type, index, rhs_type, op_ == BinOp::Sub);
    gen.Store(ir::IrType::Ptr, result, address);
    return result;
}

Operand IncDec::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    IrType ir_type = IRGen::ToIr(*type);
    Operand address = operand_->EmitIRAddress(gen);
    Operand old = gen.Load(ir_type, address);
    Operand updated = type->IsPointer()    ? StepPointer(gen, old, *type)
                      : type->IsFloating() ? StepFloat(gen, old, *type)
                                           : StepInteger(gen, old, type);
    gen.Store(ir_type, updated, address);
    return prefix_ ? updated : old;
}

// ++ and -- on a pointer move it by one element.
Operand IncDec::StepPointer(IRGen& gen, Operand old, const Type& type) const {
    Operand one = Operand::Int(1, IRGen::ToIr(*Type::Int()));
    return PointerOffset(gen, old, type, one, *Type::Int(), !increment_);
}

// A float or double is stepped by 1.0 in its own type.
Operand IncDec::StepFloat(IRGen& gen, Operand old, const Type& type) const {
    Operand one = gen.FloatConstant(1.0, old.type);
    return Arith(gen, increment_ ? BinOp::Add : BinOp::Sub, type, old, one);
}

// An integer is promoted, stepped by one, and converted back.
Operand IncDec::StepInteger(IRGen& gen, Operand old, const TypePtr& type) const {
    RequireIrInteger(*type);
    TypePtr wide = Promote(type);
    Operand step = Operand::Int(increment_ ? 1 : -1, IRGen::ToIr(*wide));
    Operand sum = Arith(gen, BinOp::Add, *wide, gen.ConvertInt(old, *type, *wide), step);
    return gen.ConvertInt(sum, *wide, *type);
}

}  // namespace ast
