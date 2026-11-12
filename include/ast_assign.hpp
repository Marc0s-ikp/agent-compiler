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
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ir::Operand EmitRecordCopy(IRGen& gen, const Type& type) const;

    ExprPtr lhs_, rhs_;
};

// lhs op= rhs. The address of lhs is evaluated once.
class CompoundAssign : public Expr {
public:
    CompoundAssign(BinOp op, ExprPtr lhs, ExprPtr rhs)
        : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
    TypePtr GetType(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ir::Operand EmitPointerValue(IRGen& gen, const Type& type, const Type& rhs_type) const;

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
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ir::Operand StepPointer(IRGen& gen, ir::Operand old, const Type& type) const;
    ir::Operand StepFloat(IRGen& gen, ir::Operand old, const Type& type) const;
    ir::Operand StepInteger(IRGen& gen, ir::Operand old, const TypePtr& type) const;

    bool increment_, prefix_;
    ExprPtr operand_;
};

}  // namespace ast
