#pragma once
#include "ast_node.hpp"

namespace ast {

enum class BinOp { Add, Sub, Mul, Div, Mod, Shl, Shr, And, Or, Xor };
enum class CmpOp { Lt, Gt, Le, Ge, Eq, Ne };
enum class UnOp { Plus, Neg, BitNot, LogNot };

// Throws unless 't' is an integer type; returns it.
TypePtr RequireInteger(const TypePtr& t);

// a0 = a0 <op> t0, using signedness of 'common' (shared with CompoundAssign).
void EmitBinOp(std::ostream& os, BinOp op, const Type& common);

class BinaryArith : public Expr {
public:
    BinaryArith(BinOp op, ExprPtr lhs, ExprPtr rhs)
        : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    BinOp op_;
    ExprPtr lhs_, rhs_;
};

class BinaryCompare : public Expr {
public:
    BinaryCompare(CmpOp op, ExprPtr lhs, ExprPtr rhs)
        : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    CmpOp op_;
    ExprPtr lhs_, rhs_;
};

class LogicalAnd : public Expr {
public:
    LogicalAnd(ExprPtr lhs, ExprPtr rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr lhs_, rhs_;
};

class LogicalOr : public Expr {
public:
    LogicalOr(ExprPtr lhs, ExprPtr rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr lhs_, rhs_;
};

class UnaryOp : public Expr {
public:
    UnaryOp(UnOp op, ExprPtr operand)
        : op_(op), operand_(std::move(operand)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

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
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr cond_, then_, else_;
};

class Comma : public Expr {
public:
    Comma(ExprPtr lhs, ExprPtr rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr lhs_, rhs_;
};

}  // namespace ast
