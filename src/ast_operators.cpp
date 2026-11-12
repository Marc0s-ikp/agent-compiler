#include "ast_operators.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

#include "const_value.hpp"
#include "target.hpp"

namespace ast {

TypePtr RequireInteger(const TypePtr& t) {
    if (!t->IsInteger()) throw std::runtime_error("integer operand required");
    return t;
}

TypePtr RequireScalar(const TypePtr& t) {
    if (!t->IsScalar()) throw std::runtime_error("scalar operand required");
    return t;
}

TypePtr RequireArithmetic(const TypePtr& t) {
    if (!t->IsArithmetic()) throw std::runtime_error("arithmetic operand required");
    return t;
}

namespace {

// The operators defined on floating operands.
bool IsRealOp(BinOp op) {
    return op == BinOp::Add || op == BinOp::Sub || op == BinOp::Mul ||
           op == BinOp::Div;
}

}  // namespace

int PointeeSize(const Type& pointer) {
    int size = pointer.Base()->Size();
    if (size == 0) throw std::runtime_error("arithmetic on pointer to void");
    return size;
}

PointerOp ClassifyPointerOp(BinOp op, const TypePtr& l, const TypePtr& r) {
    if (!l->IsPointer() && !r->IsPointer()) return PointerOp::None;
    if (op == BinOp::Add) {
        if (l->IsPointer() && r->IsInteger()) return PointerOp::PtrPlusInt;
        if (l->IsInteger() && r->IsPointer()) return PointerOp::IntPlusPtr;
    }
    if (op == BinOp::Sub) {
        if (l->IsPointer() && r->IsInteger()) return PointerOp::PtrMinusInt;
        if (l->IsPointer() && r->IsPointer()) {
            if (!l->Base()->Equals(*r->Base())) {
                throw std::runtime_error("subtracting incompatible pointers");
            }
            return PointerOp::PtrMinusPtr;
        }
    }
    throw std::runtime_error("invalid operands to binary operator");
}

namespace {

double FoldReal(BinOp op, double a, double b) {
    switch (op) {
        case BinOp::Add: return a + b;
        case BinOp::Sub: return a - b;
        case BinOp::Mul: return a * b;
        default: return a / b;
    }
}

bool IsShift(BinOp op) { return op == BinOp::Shl || op == BinOp::Shr; }

std::optional<long> FoldBinary(BinOp op, long a, long b) {
    switch (op) {
        case BinOp::Add: return a + b;
        case BinOp::Sub: return a - b;
        case BinOp::Mul:  // unsigned operands reach 2^32: multiply without signed overflow
            return static_cast<long>(static_cast<uint64_t>(a) * static_cast<uint64_t>(b));
        case BinOp::Div: return b == 0 ? std::nullopt : std::optional<long>(a / b);
        case BinOp::Mod: return b == 0 ? std::nullopt : std::optional<long>(a % b);
        case BinOp::Shl: return (b < 0 || b > 31) ? std::nullopt : std::optional<long>(a << b);
        case BinOp::Shr: return (b < 0 || b > 31) ? std::nullopt : std::optional<long>(a >> b);
        case BinOp::And: return a & b;
        case BinOp::Or: return a | b;
        case BinOp::Xor: return a ^ b;
    }
    return std::nullopt;
}

// The one place integer binary folding happens. Both operands are canonical
// (WrapToType form); they are converted to the types the instruction would
// see, folded on long, and the result is wrapped to the result type, so the
// value matches what the run-time code computes.
std::optional<long> FoldIntBinary(BinOp op, long a, long b, const Type& result,
                                  const Type& rhs_type) {
    std::optional<long> n =
        FoldBinary(op, WrapToType(a, result), WrapToType(b, rhs_type));
    if (!n) return std::nullopt;
    return WrapToType(*n, result);
}

}  // namespace

// ---- BinaryArith ----

TypePtr BinaryArith::GetType(const Context& ctx) const {
    TypePtr l = lhs_->ValueType(ctx);
    TypePtr r = rhs_->ValueType(ctx);
    switch (ClassifyPointerOp(op_, l, r)) {
        case PointerOp::PtrPlusInt:
        case PointerOp::PtrMinusInt: return l;
        case PointerOp::IntPlusPtr: return r;
        case PointerOp::PtrMinusPtr: return Type::Int();
        case PointerOp::None: break;
    }
    RequireArithmetic(l);
    RequireArithmetic(r);
    if (l->IsFloating() || r->IsFloating()) {
        if (!IsRealOp(op_)) throw std::runtime_error("integer operand required");
        return UsualArithmetic(l, r);
    }
    // The result of a shift has the promoted type of the left operand only.
    if (IsShift(op_)) return Promote(l);
    return UsualArithmetic(l, r);
}

std::optional<long> BinaryArith::EvalConst(const Context& ctx) const {
    std::optional<long> a = lhs_->EvalConst(ctx);
    std::optional<long> b = rhs_->EvalConst(ctx);
    if (!a || !b) return std::nullopt;
    TypePtr result = GetType(ctx);
    if (!result->IsInteger()) return std::nullopt;
    TypePtr rhs_type = IsShift(op_) ? Promote(rhs_->ValueType(ctx)) : result;
    return FoldIntBinary(op_, *a, *b, *result, *rhs_type);
}

// Integer constants fold as usual; 'address +/- integer' keeps the label and
// moves the offset by whole elements.
std::optional<ConstValue> BinaryArith::EvalConstValue(
    Context& ctx) const {
    if (std::optional<long> n = EvalConst(ctx)) return ConstValue{"", *n};
    PointerOp kind = ClassifyPointerOp(op_, lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    if (kind == PointerOp::None) return FoldArithmetic(ctx);
    if (kind == PointerOp::PtrMinusPtr) return std::nullopt;
    bool pointer_first = kind != PointerOp::IntPlusPtr;
    const Expr& pointer = pointer_first ? *lhs_ : *rhs_;
    const Expr& index = pointer_first ? *rhs_ : *lhs_;
    std::optional<ConstValue> base = pointer.EvalConstValue(ctx);
    std::optional<long> n = index.EvalConst(ctx);
    if (!base || !n) return std::nullopt;
    // Pointer-width arithmetic, as the run-time pointer add does (D20).
    TypePtr ptr_int = Type::Integer(Target().pointer_size, false);
    uint64_t step = static_cast<uint64_t>(PointeeSize(*pointer.ValueType(ctx))) *
                    static_cast<uint64_t>(*n);
    uint64_t offset = static_cast<uint64_t>(base->offset);
    offset += (kind == PointerOp::PtrMinusInt) ? 0u - step : step;
    base->offset = WrapToType(static_cast<long>(offset),
                              base->label.empty() ? *ptr_int : *Type::Integer(Target().pointer_size, true));
    return base;
}

// A constant the integer-only EvalConst cannot see: floating arithmetic, or
// integer arithmetic on a cast of a floating constant. Both operands are
// converted to the result type first, as at run time.
std::optional<ConstValue> BinaryArith::FoldArithmetic(Context& ctx) const {
    TypePtr result = GetType(ctx);
    TypePtr rhs_type = IsShift(op_) ? Promote(rhs_->ValueType(ctx)) : result;
    std::optional<ConstValue> a = ConstOperandAs(ctx, *lhs_, *result);
    std::optional<ConstValue> b = ConstOperandAs(ctx, *rhs_, *rhs_type);
    if (!a || !b || !a->label.empty() || !b->label.empty()) return std::nullopt;
    if (!result->IsFloating()) {
        std::optional<long> n =
            FoldIntBinary(op_, a->offset, b->offset, *result, *rhs_type);
        if (!n) return std::nullopt;
        return ConstValue{"", *n};
    }
    ConstValue out;
    out.is_real = true;
    out.real = FoldReal(op_, a->real, b->real);
    return ConvertConst(out, *result);
}

// ---- BinaryCompare ----

// Pointers compare as unsigned numbers; the usual arithmetic conversions
// apply when both operands are arithmetic.
static TypePtr CompareType(const TypePtr& l, const TypePtr& r) {
    RequireScalar(l);
    RequireScalar(r);
    if (l->IsPointer() || r->IsPointer()) {
        if (l->IsFloating() || r->IsFloating()) {
            throw std::runtime_error("comparison of pointer and floating value");
        }
        return Type::UInt();
    }
    return UsualArithmetic(l, r);
}

TypePtr BinaryCompare::GetType(const Context& ctx) const {
    CompareType(lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    return Type::Int();
}

template <typename T>
static bool Compare(CmpOp op, T a, T b) {
    switch (op) {
        case CmpOp::Lt: return a < b;
        case CmpOp::Gt: return a > b;
        case CmpOp::Le: return a <= b;
        case CmpOp::Ge: return a >= b;
        case CmpOp::Eq: return a == b;
        case CmpOp::Ne: return a != b;
    }
    return false;
}

// Integer operands only: canonical values converted to the common type
// compare correctly as long, signed or unsigned.
std::optional<long> BinaryCompare::EvalConst(const Context& ctx) const {
    std::optional<long> a = lhs_->EvalConst(ctx);
    std::optional<long> b = rhs_->EvalConst(ctx);
    if (!a || !b) return std::nullopt;
    TypePtr common = CompareType(lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    if (!common->IsInteger() && !common->IsPointer()) return std::nullopt;
    return Compare(op_, WrapToType(*a, *common), WrapToType(*b, *common)) ? 1 : 0;
}

std::optional<ConstValue> BinaryCompare::EvalConstValue(Context& ctx) const {
    TypePtr common = CompareType(lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    std::optional<ConstValue> a = ConstOperandAs(ctx, *lhs_, *common);
    std::optional<ConstValue> b = ConstOperandAs(ctx, *rhs_, *common);
    if (!a || !b || !a->label.empty() || !b->label.empty()) return std::nullopt;
    bool result = common->IsFloating() ? Compare(op_, a->real, b->real)
                                       : Compare(op_, a->offset, b->offset);
    return ConstValue{"", result ? 1 : 0};
}

// ---- LogicalAnd / LogicalOr: evaluate only what C requires ----

TypePtr LogicalAnd::GetType(const Context&) const { return Type::Int(); }

TypePtr LogicalOr::GetType(const Context&) const { return Type::Int(); }

// ---- UnaryOp ----

TypePtr UnaryOp::GetType(const Context& ctx) const {
    TypePtr t = operand_->ValueType(ctx);
    if (op_ == UnOp::LogNot) {
        RequireScalar(t);
        return Type::Int();
    }
    if (t->IsFloating() && (op_ == UnOp::Plus || op_ == UnOp::Neg)) return t;
    return Promote(RequireInteger(t));
}

namespace {
// The operand is converted to the result type, folded, and wrapped. '!' tests
// the converted operand and yields an int.
long FoldUnary(UnOp op, long v, const Type& result) {
    long a = WrapToType(v, result);
    switch (op) {
        case UnOp::Plus: return a;
        case UnOp::Neg: return WrapToType(-a, result);
        case UnOp::BitNot: return WrapToType(~a, result);
        case UnOp::LogNot: return a == 0 ? 1 : 0;
    }
    return a;
}
}  // namespace

std::optional<long> UnaryOp::EvalConst(const Context& ctx) const {
    std::optional<long> v = operand_->EvalConst(ctx);
    if (!v) return std::nullopt;
    TypePtr result = GetType(ctx);
    if (!result->IsInteger()) return std::nullopt;
    return FoldUnary(op_, *v, *result);
}

// Also the constants EvalConst cannot see: a floating operand, or a cast of
// one.
std::optional<ConstValue> UnaryOp::EvalConstValue(Context& ctx) const {
    if (std::optional<long> n = EvalConst(ctx)) return ConstValue{"", *n};
    TypePtr result = GetType(ctx);
    if (op_ == UnOp::LogNot) {
        std::optional<ConstValue> v = operand_->EvalConstValue(ctx);
        if (!v || !v->label.empty()) return std::nullopt;
        bool zero = v->is_real ? v->real == 0 : v->offset == 0;
        return ConstValue{"", zero ? 1 : 0};
    }
    std::optional<ConstValue> v = ConstOperandAs(ctx, *operand_, *result);
    if (!v || !v->label.empty()) return std::nullopt;
    if (result->IsFloating()) {
        if (op_ == UnOp::Neg) v->real = -v->real;
        return v;
    }
    return ConstValue{"", FoldUnary(op_, v->offset, *result)};
}

// ---- Ternary ----

namespace {

// The C rule for two pointer operands of ?: . A null pointer constant takes
// the other operand's type; 'void *' with an object pointer gives 'void *';
// otherwise the types must be identical.
TypePtr CombinePointers(const Context& ctx, const Expr& e1, const Expr& e2) {
    TypePtr a = e1.ValueType(ctx);
    TypePtr b = e2.ValueType(ctx);
    if (e1.IsNullPointerConstant(ctx)) return b;
    if (e2.IsNullPointerConstant(ctx)) return a;
    if (a->Equals(*b)) return a;
    if (a->Base()->IsVoid()) return a;
    if (b->Base()->IsVoid()) return b;
    throw std::runtime_error("incompatible pointers in ?:");
}

}  // namespace

// Integers use the usual arithmetic type; a pointer wins over an integer
// (null pointer constants), and two pointers combine by CombinePointers.
TypePtr Ternary::GetType(const Context& ctx) const {
    RequireScalar(cond_->ValueType(ctx));
    TypePtr a = then_->ValueType(ctx);
    TypePtr b = else_->ValueType(ctx);
    if (a->IsRecord() || b->IsRecord()) {
        if (!a->Equals(*b)) throw std::runtime_error("different structs in ?:");
        return a;
    }
    RequireScalar(a);
    RequireScalar(b);
    if (a->IsArithmetic() && b->IsArithmetic()) return UsualArithmetic(a, b);
    if (a->IsPointer() && b->IsPointer()) {
        return CombinePointers(ctx, *then_, *else_);
    }
    return a->IsPointer() ? a : b;
}

// ---- Comma ----

TypePtr Comma::GetType(const Context& ctx) const { return rhs_->GetType(ctx); }

using ir::IrType;
using ir::Op;
using ir::Operand;

namespace {

// The type a compare's operands are converted to. With a pointer on either
// side that is the pointer's type (an integer, such as the null constant, is
// converted to it; pointers are unsigned, so lt/le become ltu/leu). Otherwise
// the usual arithmetic type.
TypePtr CompareCommonType(const Context& ctx, const Expr& lhs, const Expr& rhs) {
    TypePtr l = lhs.ValueType(ctx);
    TypePtr r = rhs.ValueType(ctx);
    RequireIrScalar(*l);
    RequireIrScalar(*r);
    if (l->IsPointer()) return l;
    if (r->IsPointer()) return r;
    return UsualArithmetic(l, r);
}

}  // namespace

// The pointer forms of + and -: both operands are evaluated in source order,
// then combined.
Operand BinaryArith::EmitPointerValue(IRGen& gen, PointerOp kind) const {
    Context& ctx = gen.Ctx();
    TypePtr l = lhs_->ValueType(ctx), r = rhs_->ValueType(ctx);
    Operand a = lhs_->EmitValue(gen);
    Operand b = rhs_->EmitValue(gen);
    switch (kind) {
        case PointerOp::PtrPlusInt: return PointerOffset(gen, a, *l, b, *r, false);
        case PointerOp::PtrMinusInt: return PointerOffset(gen, a, *l, b, *r, true);
        case PointerOp::IntPlusPtr: return PointerOffset(gen, b, *r, a, *l, false);
        case PointerOp::PtrMinusPtr: return PointerDifference(gen, a, b, *l);
        case PointerOp::None: break;
    }
    return a;
}

Operand BinaryArith::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    PointerOp kind = ClassifyPointerOp(op_, lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    if (kind != PointerOp::None) return EmitPointerValue(gen, kind);
    if (!lhs_->ValueType(ctx)->IsArithmetic() || !rhs_->ValueType(ctx)->IsArithmetic()) {
        throw std::runtime_error("invalid operands to binary operator");
    }
    TypePtr result = GetType(ctx);
    bool shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr rhs_type = shift ? Promote(rhs_->ValueType(ctx)) : result;
    Operand a = ValueAs(gen, *lhs_, *result);
    Operand b = ValueAs(gen, *rhs_, *rhs_type);
    return Arith(gen, op_, *result, a, b);
}

namespace {

// The IR has feq, flt and fle only: > and >= swap the operands, and != is
// "feq is false", which is true for NaN as C requires.
Operand EmitFloatCompare(IRGen& gen, CmpOp op, IrType type, Operand a, Operand b) {
    switch (op) {
        case CmpOp::Lt: return gen.Compare(Op::FLt, type, a, b);
        case CmpOp::Gt: return gen.Compare(Op::FLt, type, b, a);
        case CmpOp::Le: return gen.Compare(Op::FLe, type, a, b);
        case CmpOp::Ge: return gen.Compare(Op::FLe, type, b, a);
        case CmpOp::Eq: return gen.Compare(Op::FEq, type, a, b);
        case CmpOp::Ne: break;
    }
    Operand equal = gen.Compare(Op::FEq, type, a, b);
    return gen.Compare(Op::Eq, equal.type, equal, Operand::Int(0, equal.type));
}

}  // namespace

Operand BinaryCompare::EmitValue(IRGen& gen) const {
    TypePtr common = CompareCommonType(gen.Ctx(), *lhs_, *rhs_);
    Operand a = ValueAs(gen, *lhs_, *common);
    Operand b = ValueAs(gen, *rhs_, *common);
    IrType type = IRGen::ToIr(*common);
    if (common->IsFloating()) return EmitFloatCompare(gen, op_, type, a, b);
    bool s = common->IsSigned();
    Op less = s ? Op::Lt : Op::LtU, less_eq = s ? Op::Le : Op::LeU;
    switch (op_) {
        case CmpOp::Lt: return gen.Compare(less, type, a, b);
        case CmpOp::Gt: return gen.Compare(less, type, b, a);
        case CmpOp::Le: return gen.Compare(less_eq, type, a, b);
        case CmpOp::Ge: return gen.Compare(less_eq, type, b, a);
        case CmpOp::Eq: return gen.Compare(Op::Eq, type, a, b);
        case CmpOp::Ne: return gen.Compare(Op::Ne, type, a, b);
    }
    return a;
}

Operand UnaryOp::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr operand_type = operand_->ValueType(ctx);
    if (op_ == UnOp::LogNot) {  // !x is x == 0 (a pointer as its integer value)
        Operand value = Condition(gen, *operand_);
        return gen.Compare(Op::Eq, value.type, value, Operand::Int(0, value.type));
    }
    RequireIrScalar(*operand_type);
    TypePtr result = GetType(ctx);
    Operand v = ValueAs(gen, *operand_, *result);
    IrType type = IRGen::ToIr(*result);
    switch (op_) {
        case UnOp::Neg: return gen.Unary(Op::Neg, type, v);
        case UnOp::BitNot:
            if (result->IsFloating()) throw std::runtime_error("invalid operand to ~");
            return gen.Unary(Op::Not, type, v);
        default: return v;
    }
}

Operand Comma::EmitValue(IRGen& gen) const {
    lhs_->EmitValue(gen);
    return rhs_->EmitValue(gen);
}

// ---- && || ?: (control flow that yields a value) ----
// Each arm stores its value into a hidden frame object, and one load after
// the join defines the result temp, so every temp has one definition site
// (21.10.4).

Operand LogicalAnd::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    IrType ir_type = IRGen::ToIr(*type);
    Operand slot = gen.AddFrameObject("and", *type);
    std::string rhs = gen.NewLabel("andr"), is_true = gen.NewLabel("andt");
    std::string is_false = gen.NewLabel("andf"), end = gen.NewLabel("ande");
    gen.Branch(Condition(gen, *lhs_), rhs, is_false);
    gen.Label(rhs);
    gen.Branch(Condition(gen, *rhs_), is_true, is_false);
    gen.Label(is_true);
    gen.Store(ir_type, Operand::Int(1, ir_type), gen.AddressOf(slot));
    gen.Jump(end);
    gen.Label(is_false);
    gen.Store(ir_type, Operand::Int(0, ir_type), gen.AddressOf(slot));
    gen.Label(end);
    return gen.Load(ir_type, gen.AddressOf(slot));
}

Operand LogicalOr::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    IrType ir_type = IRGen::ToIr(*type);
    Operand slot = gen.AddFrameObject("or", *type);
    std::string rhs = gen.NewLabel("orr"), is_true = gen.NewLabel("ort");
    std::string is_false = gen.NewLabel("orf"), end = gen.NewLabel("ore");
    gen.Branch(Condition(gen, *lhs_), is_true, rhs);
    gen.Label(rhs);
    gen.Branch(Condition(gen, *rhs_), is_true, is_false);
    gen.Label(is_false);
    gen.Store(ir_type, Operand::Int(0, ir_type), gen.AddressOf(slot));
    gen.Jump(end);
    gen.Label(is_true);
    gen.Store(ir_type, Operand::Int(1, ir_type), gen.AddressOf(slot));
    gen.Label(end);
    return gen.Load(ir_type, gen.AddressOf(slot));
}

Operand Ternary::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    RequireIrValue(*type);
    IrType ir_type = IRGen::ToIr(*type);
    // A struct arm yields the address of its object, so the slot holds a pointer.
    Operand slot = gen.AddFrameObject("sel", type->IsRecord() ? *Type::Pointer(type) : *type);
    std::string then_label = gen.NewLabel("qthen"), else_label = gen.NewLabel("qelse");
    std::string end = gen.NewLabel("qend");
    gen.Branch(Condition(gen, *cond_), then_label, else_label);
    gen.Label(then_label);
    gen.Store(ir_type, ValueAs(gen, *then_, *type), gen.AddressOf(slot));
    gen.Jump(end);
    gen.Label(else_label);
    gen.Store(ir_type, ValueAs(gen, *else_, *type), gen.AddressOf(slot));
    gen.Label(end);
    return gen.Load(ir_type, gen.AddressOf(slot));
}

}  // namespace ast
