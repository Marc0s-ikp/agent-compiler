#include "ast_operators.hpp"

#include <stdexcept>

#include "emit.hpp"

namespace ast {

TypePtr RequireInteger(const TypePtr& t) {
    if (!t->IsInteger()) throw std::runtime_error("integer operand required");
    return t;
}

TypePtr RequireScalar(const TypePtr& t) {
    if (!t->IsScalar()) throw std::runtime_error("scalar operand required");
    return t;
}

void EmitCondition(std::ostream& os, Context& ctx, const Expr& cond) {
    RequireScalar(cond.ValueType(ctx));
    cond.EmitRISC(os, ctx);
}

void EmitBinOp(std::ostream& os, BinOp op, const Type& common) {
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

// Leaves lhs in a0 and rhs in t0, each converted to its given type.
// The lhs waits in a frame temp while the rhs is evaluated (D1).
void EmitOperands(std::ostream& os, Context& ctx, const Expr& lhs,
                  const TypePtr& lhs_to, const Expr& rhs,
                  const TypePtr& rhs_to) {
    lhs.EmitRISC(os, ctx);
    EmitConvert(os, *lhs.ValueType(ctx), *lhs_to);
    int slot = ctx.PushTemp(lhs_to);
    EmitStore(os, *lhs_to, "a0", "s0", slot);
    rhs.EmitRISC(os, ctx);
    EmitConvert(os, *rhs.ValueType(ctx), *rhs_to);
    os << "\tmv t0, a0\n";
    EmitLoad(os, *lhs_to, "a0", "s0", slot);
    ctx.PopTemp();
}

bool IsShift(BinOp op) { return op == BinOp::Shl || op == BinOp::Shr; }

std::optional<long> FoldBinary(BinOp op, long a, long b) {
    switch (op) {
        case BinOp::Add: return a + b;
        case BinOp::Sub: return a - b;
        case BinOp::Mul: return a * b;
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
    RequireInteger(l);
    RequireInteger(r);
    // The result of a shift has the promoted type of the left operand only.
    if (IsShift(op_)) return Promote(l);
    return UsualArithmetic(l, r);
}

std::optional<long> BinaryArith::EvalConst(const Context& ctx) const {
    std::optional<long> a = lhs_->EvalConst(ctx);
    std::optional<long> b = rhs_->EvalConst(ctx);
    if (!a || !b) return std::nullopt;
    return FoldBinary(op_, *a, *b);
}

// Integer constants fold as usual; 'address +/- integer' keeps the label and
// moves the offset by whole elements.
std::optional<ConstValue> BinaryArith::EvalConstValue(
    const Context& ctx) const {
    if (std::optional<long> n = EvalConst(ctx)) return ConstValue{"", *n};
    PointerOp kind = ClassifyPointerOp(op_, lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    if (kind == PointerOp::None || kind == PointerOp::PtrMinusPtr) {
        return std::nullopt;
    }
    bool pointer_first = kind != PointerOp::IntPlusPtr;
    const Expr& pointer = pointer_first ? *lhs_ : *rhs_;
    const Expr& index = pointer_first ? *rhs_ : *lhs_;
    std::optional<ConstValue> base = pointer.EvalConstValue(ctx);
    std::optional<long> n = index.EvalConst(ctx);
    if (!base || !n) return std::nullopt;
    long step = PointeeSize(*pointer.ValueType(ctx)) * *n;
    base->offset += (kind == PointerOp::PtrMinusInt) ? -step : step;
    return base;
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

// Pointers compare as unsigned numbers; usual arithmetic conversions apply
// only when both operands are integers.
static TypePtr CompareType(const TypePtr& l, const TypePtr& r) {
    RequireScalar(l);
    RequireScalar(r);
    if (l->IsPointer() || r->IsPointer()) return Type::UInt();
    return UsualArithmetic(l, r);
}

TypePtr BinaryCompare::GetType(const Context& ctx) const {
    CompareType(lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    return Type::Int();
}

void BinaryCompare::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr common = CompareType(lhs_->ValueType(ctx), rhs_->ValueType(ctx));
    EmitOperands(os, ctx, *lhs_, common, *rhs_, common);
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
    return Promote(RequireInteger(t));
}

std::optional<long> UnaryOp::EvalConst(const Context& ctx) const {
    std::optional<long> v = operand_->EvalConst(ctx);
    if (!v) return std::nullopt;
    switch (op_) {
        case UnOp::Plus: return *v;
        case UnOp::Neg: return -*v;
        case UnOp::BitNot: return ~*v;
        case UnOp::LogNot: return *v == 0 ? 1 : 0;
    }
    return std::nullopt;
}

void UnaryOp::EmitRISC(std::ostream& os, Context& ctx) const {
    operand_->EmitRISC(os, ctx);
    EmitConvert(os, *operand_->ValueType(ctx), *GetType(ctx));
    switch (op_) {
        case UnOp::Plus: break;
        case UnOp::Neg: os << "\tneg a0, a0\n"; break;
        case UnOp::BitNot: os << "\tnot a0, a0\n"; break;
        case UnOp::LogNot: os << "\tseqz a0, a0\n"; break;
    }
}

// ---- Ternary ----

// Integers use the usual arithmetic type; a pointer wins over an integer
// (null pointer constants), and two pointers must agree.
TypePtr Ternary::GetType(const Context& ctx) const {
    RequireScalar(cond_->ValueType(ctx));
    TypePtr a = RequireScalar(then_->ValueType(ctx));
    TypePtr b = RequireScalar(else_->ValueType(ctx));
    if (a->IsInteger() && b->IsInteger()) return UsualArithmetic(a, b);
    if (a->IsPointer() && b->IsPointer()) {
        if (!a->Equals(*b)) throw std::runtime_error("incompatible pointers in ?:");
        return a;
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

}  // namespace ast
