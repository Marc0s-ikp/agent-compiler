// Lowering helpers shared by several nodes' EmitIR / EmitValue (D23 a). The
// lowering of each node lives next to its EmitRISC; only what more than one
// node needs is here.
#include "ast_lower.hpp"

#include <stdexcept>

#include "const_value.hpp"
#include "target.hpp"

namespace ast {

using ir::Op;
using ir::Operand;

const Type& RequireIrInteger(const Type& type) {
    if (!type.IsInteger()) throw std::runtime_error("not in IR yet: non-integer operand");
    return type;
}

const Type& RequireIrScalar(const Type& type) {
    if (!type.IsInteger() && !type.IsPointer()) throw std::runtime_error("not in IR yet: this type");
    return type;
}

namespace {

Op ArithOp(BinOp op, bool is_signed) {
    switch (op) {
        case BinOp::Add: return Op::Add;
        case BinOp::Sub: return Op::Sub;
        case BinOp::Mul: return Op::Mul;
        case BinOp::Div: return is_signed ? Op::Div : Op::DivU;
        case BinOp::Mod: return is_signed ? Op::Rem : Op::RemU;
        case BinOp::Shl: return Op::Shl;
        case BinOp::Shr: return is_signed ? Op::Sar : Op::Shr;
        case BinOp::And: return Op::And;
        case BinOp::Or: return Op::Or;
        case BinOp::Xor: return Op::Xor;
    }
    return Op::Add;
}

}  // namespace

// a <op> b, both already converted to 'common'.
Operand Arith(IRGen& gen, BinOp op, const Type& common, Operand a, Operand b) {
    return gen.Binary(ArithOp(op, common.IsSigned()), IRGen::ToIr(common), a, b);
}

// Integer to pointer: first to the pointer's width, extending by the source's
// signedness (sext for signed, zext for unsigned; the same width needs
// nothing, the bits are the same), then inttoptr. Pointer to integer is the
// reverse, with the pointer taken as unsigned. A null pointer constant goes
// through the same path: inttoptr of 0.
Operand ConvertScalar(IRGen& gen, Operand value, const Type& from, const Type& to) {
    RequireIrScalar(from);
    RequireIrScalar(to);
    int width = Target().pointer_size;
    if (from.IsPointer() && to.IsPointer()) return value;
    if (to.IsPointer()) {
        TypePtr wide = Type::Integer(width, from.IsSigned());
        return gen.IntToPtr(gen.ConvertInt(value, from, *wide));
    }
    if (from.IsPointer()) {
        TypePtr wide = Type::Integer(width, false);
        return gen.ConvertInt(gen.PtrToInt(value), *wide, to);
    }
    return gen.ConvertInt(value, from, to);
}

Operand ValueAs(IRGen& gen, const Expr& e, const Type& to) {
    TypePtr from = e.ValueType(gen.Ctx());
    return ConvertScalar(gen, e.EmitValue(gen), *from, to);
}

Operand Condition(IRGen& gen, const Expr& e) {
    TypePtr type = e.ValueType(gen.Ctx());
    RequireIrScalar(*type);
    Operand value = e.EmitValue(gen);
    return type->IsPointer() ? gen.PtrToInt(value) : value;
}

namespace {

// n * size, folded when n is a constant; the type is the pointer-width integer.
Operand Scale(IRGen& gen, Operand n, int size, const Type& wide) {
    if (n.kind == Operand::Kind::Int) return Operand::Int(WrapToType(n.int_value * size, wide), n.type);
    if (size == 1) return n;
    return gen.Binary(Op::Mul, n.type, n, Operand::Int(size, n.type));
}

Operand Negate(IRGen& gen, Operand n, const Type& wide) {
    if (n.kind == Operand::Kind::Int) return Operand::Int(WrapToType(-n.int_value, wide), n.type);
    return gen.Unary(Op::Neg, n.type, n);
}

}  // namespace

Operand PointerOffset(IRGen& gen, Operand pointer, const Type& pointer_type, Operand index,
                      const Type& index_type, bool subtract) {
    RequireIrInteger(index_type);
    int size = PointeeSize(pointer_type);
    TypePtr wide = Type::Integer(Target().pointer_size, index_type.IsSigned());
    Operand n = Scale(gen, gen.ConvertInt(index, index_type, *wide), size, *wide);
    if (subtract) n = Negate(gen, n, *wide);
    return gen.PtrAdd(pointer, n);
}

Operand PointerDifference(IRGen& gen, Operand a, Operand b, const Type& pointer_type) {
    int size = PointeeSize(pointer_type);
    TypePtr wide = Type::Integer(Target().pointer_size, true);
    Operand bytes = gen.Binary(Op::Sub, ir::PointerIntType(), gen.PtrToInt(a), gen.PtrToInt(b));
    Operand count = size == 1 ? bytes
                              : gen.Binary(Op::Div, ir::PointerIntType(), bytes,
                                           Operand::Int(size, ir::PointerIntType()));
    return gen.ConvertInt(count, *wide, *Type::Int());
}

}  // namespace ast
