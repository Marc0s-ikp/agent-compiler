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
    if (!type.IsScalar()) throw std::runtime_error("not in IR yet: this type");
    return type;
}

const Type& RequireIrValue(const Type& type) {
    if (!type.IsScalar() && !type.IsRecord()) throw std::runtime_error("not in IR yet: this type");
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
    if (common.IsFloating()) {  // signedness means nothing here; no % or bit operations
        if (op != BinOp::Add && op != BinOp::Sub && op != BinOp::Mul && op != BinOp::Div) {
            throw std::runtime_error("invalid operands to binary operator");
        }
        return gen.Binary(ArithOp(op, true), IRGen::ToIr(common), a, b);
    }
    return gen.Binary(ArithOp(op, common.IsSigned()), IRGen::ToIr(common), a, b);
}

// A conversion with a floating type on either side. float <-> double is
// fext/ftrunc. An integer goes to float through a 32-bit int (a narrow one is
// extended first), signed with itof and unsigned with utof. Float to integer
// rounds toward zero: ftou for a 32-bit unsigned target, ftoi otherwise, then
// a narrow target is truncated from the int (as the old path does).
Operand ConvertFloating(IRGen& gen, Operand value, const Type& from, const Type& to) {
    if (from.IsPointer() || to.IsPointer()) throw std::runtime_error("invalid type conversion");
    if (from.IsFloating() && to.IsFloating()) {
        if (from.Size() == to.Size()) return value;
        return gen.Convert(to.Size() > from.Size() ? Op::FExt : Op::FTrunc, IRGen::ToIr(to), value);
    }
    if (to.IsFloating()) {
        TypePtr wide = Type::Integer(Target().int_size, from.IsSigned());
        Operand word = from.Size() < wide->Size() ? gen.ConvertInt(value, from, *wide) : value;
        return gen.Convert(from.IsSigned() ? Op::IToF : Op::UToF, IRGen::ToIr(to), word);
    }
    bool unsigned_word = !to.IsSigned() && to.Size() == Target().int_size;
    TypePtr word_type = unsigned_word ? Type::UInt() : Type::Int();
    Operand word = gen.Convert(unsigned_word ? Op::FToU : Op::FToI, IRGen::ToIr(*word_type), value);
    return to.Size() < word_type->Size() ? gen.ConvertInt(word, *word_type, to) : word;
}

// Integer to pointer: first to the pointer's width, extending by the source's
// signedness (sext for signed, zext for unsigned; the same width needs
// nothing, the bits are the same), then inttoptr. Pointer to integer is the
// reverse, with the pointer taken as unsigned. A null pointer constant goes
// through the same path: inttoptr of 0.
Operand ConvertScalar(IRGen& gen, Operand value, const Type& from, const Type& to) {
    RequireIrScalar(from);
    RequireIrScalar(to);
    if (from.IsFloating() || to.IsFloating()) return ConvertFloating(gen, value, from, to);
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
    if (to.IsRecord() || from->IsRecord()) {  // only the same struct or union converts
        if (!from->Equals(to)) throw std::runtime_error("invalid type conversion");
        return e.EmitValue(gen);
    }
    return ConvertScalar(gen, e.EmitValue(gen), *from, to);
}

// A floating condition is "not equal to 0.0", which is true for NaN: feq is
// false for NaN, so the test is feq(x, 0.0) == 0.
Operand FloatCondition(IRGen& gen, Operand value) {
    Operand zero = gen.FloatConstant(0.0, value.type);
    Operand is_zero = gen.Compare(Op::FEq, value.type, value, zero);
    return gen.Compare(Op::Eq, is_zero.type, is_zero, Operand::Int(0, is_zero.type));
}

Operand Condition(IRGen& gen, const Expr& e) {
    TypePtr type = e.ValueType(gen.Ctx());
    RequireIrScalar(*type);
    Operand value = e.EmitValue(gen);
    if (type->IsFloating()) return FloatCondition(gen, value);
    return type->IsPointer() ? gen.PtrToInt(value) : value;
}

Operand LoadObject(IRGen& gen, Operand address, const Type& type) {
    if (type.IsArray() || type.IsRecord()) return address;
    RequireIrScalar(type);
    return gen.Load(IRGen::ToIr(type), address);
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
