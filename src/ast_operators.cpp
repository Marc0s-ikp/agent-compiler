#include "ast_operators.hpp"

#include <stdexcept>

#include "emit.hpp"

namespace ast {

TypePtr RequireInteger(const TypePtr& t) {
    if (!t->IsInteger()) throw std::runtime_error("integer operand required");
    return t;
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

// Leaves lhs in a0 and rhs in t0, each converted to its given type.
// The lhs waits in a frame temp while the rhs is evaluated (D1).
void EmitOperands(std::ostream& os, Context& ctx, const Expr& lhs,
                  const TypePtr& lhs_to, const Expr& rhs,
                  const TypePtr& rhs_to) {
    lhs.EmitRISC(os, ctx);
    EmitConvert(os, *lhs.GetType(ctx), *lhs_to);
    int slot = ctx.PushTemp(lhs_to);
    EmitStore(os, *lhs_to, "a0", "s0", slot);
    rhs.EmitRISC(os, ctx);
    EmitConvert(os, *rhs.GetType(ctx), *rhs_to);
    os << "\tmv t0, a0\n";
    EmitLoad(os, *lhs_to, "a0", "s0", slot);
    ctx.PopTemp();
}

}  // namespace

// ---- BinaryArith ----

TypePtr BinaryArith::GetType(const Context& ctx) const {
    TypePtr l = RequireInteger(lhs_->GetType(ctx));
    TypePtr r = RequireInteger(rhs_->GetType(ctx));
    // The result of a shift has the promoted type of the left operand only.
    if (op_ == BinOp::Shl || op_ == BinOp::Shr) return Promote(l);
    return UsualArithmetic(l, r);
}

void BinaryArith::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr result = GetType(ctx);
    TypePtr rhs_type = result;
    if (op_ == BinOp::Shl || op_ == BinOp::Shr) {
        rhs_type = Promote(rhs_->GetType(ctx));
    }
    EmitOperands(os, ctx, *lhs_, result, *rhs_, rhs_type);
    EmitBinOp(os, op_, *result);
}

// ---- BinaryCompare ----

TypePtr BinaryCompare::GetType(const Context& ctx) const {
    RequireInteger(lhs_->GetType(ctx));
    RequireInteger(rhs_->GetType(ctx));
    return Type::Int();
}

void BinaryCompare::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr common = UsualArithmetic(lhs_->GetType(ctx), rhs_->GetType(ctx));
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
    lhs_->EmitRISC(os, ctx);
    EmitBranchIfZero(os, is_false);
    rhs_->EmitRISC(os, ctx);
    EmitBranchIfZero(os, is_false);
    os << "\tli a0, 1\n\tj " << end << "\n";
    os << is_false << ":\n\tli a0, 0\n" << end << ":\n";
}

TypePtr LogicalOr::GetType(const Context&) const { return Type::Int(); }

void LogicalOr::EmitRISC(std::ostream& os, Context& ctx) const {
    std::string is_true = ctx.NewLabel("ort");
    std::string end = ctx.NewLabel("ore");
    lhs_->EmitRISC(os, ctx);
    EmitBranchIfNonZero(os, is_true);
    rhs_->EmitRISC(os, ctx);
    EmitBranchIfNonZero(os, is_true);
    os << "\tli a0, 0\n\tj " << end << "\n";
    os << is_true << ":\n\tli a0, 1\n" << end << ":\n";
}

// ---- UnaryOp ----

TypePtr UnaryOp::GetType(const Context& ctx) const {
    TypePtr t = RequireInteger(operand_->GetType(ctx));
    if (op_ == UnOp::LogNot) return Type::Int();
    return Promote(t);
}

void UnaryOp::EmitRISC(std::ostream& os, Context& ctx) const {
    operand_->EmitRISC(os, ctx);
    EmitConvert(os, *operand_->GetType(ctx), *GetType(ctx));
    switch (op_) {
        case UnOp::Plus: break;
        case UnOp::Neg: os << "\tneg a0, a0\n"; break;
        case UnOp::BitNot: os << "\tnot a0, a0\n"; break;
        case UnOp::LogNot: os << "\tseqz a0, a0\n"; break;
    }
}

// ---- Ternary ----

TypePtr Ternary::GetType(const Context& ctx) const {
    RequireInteger(cond_->GetType(ctx));
    return UsualArithmetic(RequireInteger(then_->GetType(ctx)),
                           RequireInteger(else_->GetType(ctx)));
}

void Ternary::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr result = GetType(ctx);
    std::string else_label = ctx.NewLabel("elsq");
    std::string end = ctx.NewLabel("endq");
    cond_->EmitRISC(os, ctx);
    EmitBranchIfZero(os, else_label);
    then_->EmitRISC(os, ctx);
    EmitConvert(os, *then_->GetType(ctx), *result);
    os << "\tj " << end << "\n" << else_label << ":\n";
    else_->EmitRISC(os, ctx);
    EmitConvert(os, *else_->GetType(ctx), *result);
    os << end << ":\n";
}

// ---- Comma ----

TypePtr Comma::GetType(const Context& ctx) const { return rhs_->GetType(ctx); }

void Comma::EmitRISC(std::ostream& os, Context& ctx) const {
    lhs_->EmitRISC(os, ctx);
    rhs_->EmitRISC(os, ctx);
}

}  // namespace ast
