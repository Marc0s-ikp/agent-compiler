#pragma once
#include "ast_node.hpp"

namespace ast {

// *operand. An lvalue; its address is the operand's value.
class Deref : public Expr {
public:
    explicit Deref(ExprPtr operand) : operand_(std::move(operand)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitAddress(std::ostream& os, Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;
    ir::Operand EmitIRAddress(IRGen& gen) const override;

private:
    ExprPtr operand_;
};

// &operand. Yields a pointer to the operand's object.
class AddressOf : public Expr {
public:
    explicit AddressOf(ExprPtr operand) : operand_(std::move(operand)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ExprPtr operand_;
};

// base[index], defined as *(base + index), so index[base] also works.
class Index : public Expr {
public:
    Index(ExprPtr base, ExprPtr index);
    TypePtr GetType(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    std::optional<ConstValue> EvalConstAddress(Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitAddress(std::ostream& os, Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;
    ir::Operand EmitIRAddress(IRGen& gen) const override;

private:
    ExprPtr sum_;  // base + index
};

}  // namespace ast
