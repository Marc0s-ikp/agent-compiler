#pragma once
#include "ast_node.hpp"
#include "ast_operators.hpp"

namespace ast {

// lhs = rhs. The value of the expression is the stored value.
class Assign : public Expr {
public:
    Assign(ExprPtr lhs, ExprPtr rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr lhs_, rhs_;
};

// lhs op= rhs. The address of lhs is evaluated once.
class CompoundAssign : public Expr {
public:
    CompoundAssign(BinOp op, ExprPtr lhs, ExprPtr rhs)
        : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    BinOp op_;
    ExprPtr lhs_, rhs_;
};

// ++x, --x, x++, x--.
class IncDec : public Expr {
public:
    IncDec(bool is_increment, bool is_prefix, ExprPtr operand)
        : increment_(is_increment),
          prefix_(is_prefix),
          operand_(std::move(operand)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    void EmitFloating(std::ostream& os, Context& ctx, const Type& type) const;

    bool increment_, prefix_;
    ExprPtr operand_;
};

}  // namespace ast
