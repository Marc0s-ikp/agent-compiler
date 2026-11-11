#include "ast_operators.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

#include "const_value.hpp"
#include "emit.hpp"
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

void EmitCondition(std::ostream& os, Context& ctx, const Expr& cond) {
    TypePtr type = RequireScalar(cond.ValueType(ctx));
    cond.EmitRISC(os, ctx);
    if (type->IsFloating()) EmitFloatTruth(os, *type);
}

namespace {

// The operators defined on floating operands.
bool IsRealOp(BinOp op) {
    return op == BinOp::Add || op == BinOp::Sub || op == BinOp::Mul ||
           op == BinOp::Div;
}

void EmitFloatBinOp(std::ostream& os, BinOp op, const Type& common) {
    const char* name = "fadd";
    switch (op) {
        case BinOp::Add: name = "fadd"; break;
        case BinOp::Sub: name = "fsub"; break;
        case BinOp::Mul: name = "fmul"; break;
        case BinOp::Div: name = "fdiv"; break;
        default: throw std::runtime_error("invalid operands to binary operator");
    }
    char suffix = common.Size() == 4 ? 's' : 'd';
    os << "\t" << name << "." << suffix << " fa0, fa0, ft0\n";
}

}  // namespace

void EmitBinOp(std::ostream& os, BinOp op, const Type& common) {
    if (common.IsFloating()) return EmitFloatBinOp(os, op, common);
    bool s = common.IsSigned();
    const char* name = "add";
    switch (op) {
        case BinOp::Add: name = "add"; break;
        case BinOp::Sub: name = "sub"; break;
        case BinOp::Mul: name = "mul"; break;
        case BinOp::Div: name = s ? "div" : "divu"; break;
        case BinOp::Mod: name = s ? "rem" : "remu"; break;
        case BinOp::Shl: name = "sll"; break;
        case BinOp::Shr: name = s ? "sra" : "srl"; break;
        case BinOp::And: name = "and"; break;
        case BinOp::Or: name = "or"; break;
        case BinOp::Xor: name = "xor"; break;
    }
    os << "\t" << name << " a0, a0, t0\n";
}

namespace {

bool IsPowerOfTwo(int n) { return (n & (n - 1)) == 0; }

int Log2(int n) {
    int shift = 0;
    while ((1 << shift) < n) ++shift;
    return shift;
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

void EmitPointerAdd(std::ostream& os, bool subtract, const Type& pointer) {
    int size = PointeeSize(pointer);
    if (size != 1) {
        if (IsPowerOfTwo(size)) {
            os << "\tslli t0, t0, " << Log2(size) << "\n";
        } else {
            os << "\tli t6, " << size << "\n\tmul t0, t0, t6\n";
        }
    }
    os << "\t" << (subtract ? "sub" : "add") << " a0, a0, t0\n";
}

void EmitPointerDiff(std::ostream& os, const Type& pointer) {
    int size = PointeeSize(pointer);
    os << "\tsub a0, a0, t0\n";
    if (size == 1) return;
    if (IsPowerOfTwo(size)) {
        os << "\tsrai a0, a0, " << Log2(size) << "\n";
    } else {
        os << "\tli t6, " << size << "\n\tdiv a0, a0, t6\n";
    }
}

namespace {

// Leaves lhs in the accumulator and rhs in the scratch register (a0, t0 or
// fa0, ft0 by type), each converted to its given type.
// The lhs waits in a frame temp while the rhs is evaluated (D1).
void EmitOperands(std::ostream& os, Context& ctx, const Expr& lhs,
                  const TypePtr& lhs_to, const Expr& rhs,
                  const TypePtr& rhs_to) {
    lhs.EmitRISC(os, ctx);
    EmitConvert(os, *lhs.ValueType(ctx), *lhs_to);
    int slot = ctx.PushTemp(lhs_to);
    EmitStore(os, *lhs_to, AccReg(*lhs_to), "s0", slot);
    rhs.EmitRISC(os, ctx);
    EmitConvert(os, *rhs.ValueType(ctx), *rhs_to);
    EmitMoveToScratch(os, *rhs_to);
    EmitLoad(os, *lhs_to, AccReg(*lhs_to), "s0", slot);
    ctx.PopTemp();
}

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

void BinaryArith::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr l = lhs_->ValueType(ctx);
    TypePtr r = rhs_->ValueType(ctx);
    PointerOp pointer_op = ClassifyPointerOp(op_, l, r);
    if (pointer_op != PointerOp::None) {
        EmitOperands(os, ctx, *lhs_, l, *rhs_, r);
        switch (pointer_op) {
            case PointerOp::PtrPlusInt: EmitPointerAdd(os, false, *l); break;
            case PointerOp::PtrMinusInt: EmitPointerAdd(os, true, *l); break;
            case PointerOp::PtrMinusPtr: EmitPointerDiff(os, *l); break;
            case PointerOp::IntPlusPtr:
                os << "\tmv t1, a0\n\tmv a0, t0\n\tmv t0, t1\n";
                EmitPointerAdd(os, false, *r);
                break;
            case PointerOp::None: break;
        }
        return;
    }
    TypePtr result = GetType(ctx);
    TypePtr rhs_type = IsShift(op_) ? Promote(r) : result;
    EmitOperands(os, ctx, *lhs_, result, *rhs_, rhs_type);
    EmitBinOp(os, op_, *result);
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

// NaN makes every ordered compare false and != true: Gt and Ge are flt/fle
// with the operands swapped, Ne is the inverse of feq.
static void EmitFloatCompare(std::ostream& os, CmpOp op, const Type& common) {
    std::string s = common.Size() == 4 ? ".s" : ".d";
    switch (op) {
        case CmpOp::Lt: os << "\tflt" << s << " a0, fa0, ft0\n"; break;
        case CmpOp::Gt: os << "\tflt" << s << " a0, ft0, fa0\n"; break;
        case CmpOp::Le: os << "\tfle" << s << " a0, fa0, ft0\n"; break;
        case CmpOp::Ge: os << "\tfle" << s << " a0, ft0, fa0\n"; break;
        case CmpOp::Eq: os << "\tfeq" << s << " a0, fa0, ft0\n"; break;
        case CmpOp::Ne:
            os << "\tfeq" << s << " a0, fa0, ft0\n\txori a0, a0, 1\n";
            break;
    }
}

void BinaryCompare::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr common = CompareType(lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    EmitOperands(os, ctx, *lhs_, common, *rhs_, common);
    if (common->IsFloating()) return EmitFloatCompare(os, op_, *common);
    const char* slt = common->IsSigned() ? "slt" : "sltu";
    switch (op_) {
        case CmpOp::Lt: os << "\t" << slt << " a0, a0, t0\n"; break;
        case CmpOp::Gt: os << "\t" << slt << " a0, t0, a0\n"; break;
        case CmpOp::Le:
            os << "\t" << slt << " a0, t0, a0\n\txori a0, a0, 1\n";
            break;
        case CmpOp::Ge:
            os << "\t" << slt << " a0, a0, t0\n\txori a0, a0, 1\n";
            break;
        case CmpOp::Eq: os << "\txor a0, a0, t0\n\tseqz a0, a0\n"; break;
        case CmpOp::Ne: os << "\txor a0, a0, t0\n\tsnez a0, a0\n"; break;
    }
}

// ---- LogicalAnd / LogicalOr: evaluate only what C requires ----

TypePtr LogicalAnd::GetType(const Context&) const { return Type::Int(); }

void LogicalAnd::EmitRISC(std::ostream& os, Context& ctx) const {
    std::string is_false = ctx.NewLabel("andf");
    std::string end = ctx.NewLabel("ande");
    EmitCondition(os, ctx, *lhs_);
    EmitBranchIfZero(os, is_false);
    EmitCondition(os, ctx, *rhs_);
    EmitBranchIfZero(os, is_false);
    os << "\tli a0, 1\n\tj " << end << "\n";
    os << is_false << ":\n\tli a0, 0\n" << end << ":\n";
}

TypePtr LogicalOr::GetType(const Context&) const { return Type::Int(); }

void LogicalOr::EmitRISC(std::ostream& os, Context& ctx) const {
    std::string is_true = ctx.NewLabel("ort");
    std::string end = ctx.NewLabel("ore");
    EmitCondition(os, ctx, *lhs_);
    EmitBranchIfNonZero(os, is_true);
    EmitCondition(os, ctx, *rhs_);
    EmitBranchIfNonZero(os, is_true);
    os << "\tli a0, 0\n\tj " << end << "\n";
    os << is_true << ":\n\tli a0, 1\n" << end << ":\n";
}

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

void UnaryOp::EmitRISC(std::ostream& os, Context& ctx) const {
    if (op_ == UnOp::LogNot) {
        EmitCondition(os, ctx, *operand_);
        os << "\tseqz a0, a0\n";
        return;
    }
    operand_->EmitRISC(os, ctx);
    TypePtr result = GetType(ctx);
    EmitConvert(os, *operand_->ValueType(ctx), *result);
    if (op_ == UnOp::Neg && result->IsFloating()) {
        os << "\tfneg." << (result->Size() == 4 ? 's' : 'd') << " fa0, fa0\n";
        return;
    }
    switch (op_) {
        case UnOp::Plus: break;
        case UnOp::Neg: os << "\tneg a0, a0\n"; break;
        case UnOp::BitNot: os << "\tnot a0, a0\n"; break;
        case UnOp::LogNot: break;
    }
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

void Ternary::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr result = GetType(ctx);
    std::string else_label = ctx.NewLabel("elsq");
    std::string end = ctx.NewLabel("endq");
    EmitCondition(os, ctx, *cond_);
    EmitBranchIfZero(os, else_label);
    then_->EmitRISC(os, ctx);
    EmitConvert(os, *then_->ValueType(ctx), *result);
    os << "\tj " << end << "\n" << else_label << ":\n";
    else_->EmitRISC(os, ctx);
    EmitConvert(os, *else_->ValueType(ctx), *result);
    os << end << ":\n";
}

// ---- Comma ----

TypePtr Comma::GetType(const Context& ctx) const { return rhs_->GetType(ctx); }

void Comma::EmitRISC(std::ostream& os, Context& ctx) const {
    lhs_->EmitRISC(os, ctx);
    rhs_->EmitRISC(os, ctx);
}


// ---- --ir lowering ----

using ir::IrType;
using ir::Op;
using ir::Operand;

namespace {

// The usual arithmetic type of a compare's operands (integers only for now).
TypePtr CompareCommonType(const Context& ctx, const Expr& lhs, const Expr& rhs) {
    TypePtr l = lhs.ValueType(ctx);
    TypePtr r = rhs.ValueType(ctx);
    RequireIrInteger(*l);
    RequireIrInteger(*r);
    return UsualArithmetic(l, r);
}

}  // namespace

Operand BinaryArith::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    RequireIrInteger(*lhs_->ValueType(ctx));
    RequireIrInteger(*rhs_->ValueType(ctx));
    TypePtr result = GetType(ctx);
    bool shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr rhs_type = shift ? Promote(rhs_->ValueType(ctx)) : result;
    Operand a = ValueAs(gen, *lhs_, *result);
    Operand b = ValueAs(gen, *rhs_, *rhs_type);
    return Arith(gen, op_, *result, a, b);
}

Operand BinaryCompare::EmitValue(IRGen& gen) const {
    TypePtr common = CompareCommonType(gen.Ctx(), *lhs_, *rhs_);
    Operand a = ValueAs(gen, *lhs_, *common);
    Operand b = ValueAs(gen, *rhs_, *common);
    IrType type = IRGen::ToIr(*common);
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
    RequireIrInteger(*operand_type);
    if (op_ == UnOp::LogNot) {  // !x is x == 0
        IrType type = IRGen::ToIr(*operand_type);
        return gen.Compare(Op::Eq, type, operand_->EmitValue(gen), Operand::Int(0, type));
    }
    TypePtr result = GetType(ctx);
    Operand v = ValueAs(gen, *operand_, *result);
    IrType type = IRGen::ToIr(*result);
    switch (op_) {
        case UnOp::Neg: return gen.Unary(Op::Neg, type, v);
        case UnOp::BitNot: return gen.Unary(Op::Not, type, v);
        default: return v;
    }
}

Operand Comma::EmitValue(IRGen& gen) const {
    lhs_->EmitValue(gen);
    return rhs_->EmitValue(gen);
}

}  // namespace ast
