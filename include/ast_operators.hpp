#pragma once
#include "ast_node.hpp"

namespace ast {

enum class BinOp { Add, Sub, Mul, Div, Mod, Shl, Shr, And, Or, Xor };
enum class CmpOp { Lt, Gt, Le, Ge, Eq, Ne };
enum class UnOp { Plus, Neg, BitNot, LogNot };

// Throws unless 't' is an integer type; returns it.
TypePtr RequireInteger(const TypePtr& t);

// Throws unless 't' is an integer or pointer type; returns it.
TypePtr RequireScalar(const TypePtr& t);

// Throws unless 't' is an integer or floating type; returns it.
TypePtr RequireArithmetic(const TypePtr& t);

// Size of the object a pointer points to; throws if it has none (void).
int PointeeSize(const Type& pointer);

// The pointer forms of + and -, decided once for the whole compiler.
enum class PointerOp { None, PtrPlusInt, IntPlusPtr, PtrMinusInt, PtrMinusPtr };

// Classifies 'l op r' (value types). Throws for invalid pointer combinations.
PointerOp ClassifyPointerOp(BinOp op, const TypePtr& l, const TypePtr& r);

class BinaryArith : public Expr {
public:
    BinaryArith(BinOp op, ExprPtr lhs, ExprPtr rhs)
        : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    std::optional<ConstValue> FoldArithmetic(Context& ctx) const;
    ir::Operand EmitPointerValue(IRGen& gen, PointerOp kind) const;

    BinOp op_;
    ExprPtr lhs_, rhs_;
};

class BinaryCompare : public Expr {
public:
    BinaryCompare(CmpOp op, ExprPtr lhs, ExprPtr rhs)
        : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    CmpOp op_;
    ExprPtr lhs_, rhs_;
};

class LogicalAnd : public Expr {
public:
    LogicalAnd(ExprPtr lhs, ExprPtr rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ExprPtr lhs_, rhs_;
};

class LogicalOr : public Expr {
public:
    LogicalOr(ExprPtr lhs, ExprPtr rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ExprPtr lhs_, rhs_;
};

class UnaryOp : public Expr {
public:
    UnaryOp(UnOp op, ExprPtr operand)
        : op_(op), operand_(std::move(operand)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    UnOp op_;
    ExprPtr operand_;
};

class Ternary : public Expr {
public:
    Ternary(ExprPtr cond, ExprPtr then_expr, ExprPtr else_expr)
        : cond_(std::move(cond)),
          then_(std::move(then_expr)),
          else_(std::move(else_expr)) {}
    TypePtr GetType(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ExprPtr cond_, then_, else_;
};

class Comma : public Expr {
public:
    Comma(ExprPtr lhs, ExprPtr rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ExprPtr lhs_, rhs_;
};

}  // namespace ast
