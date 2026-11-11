#pragma once
#include "ast_declaration.hpp"
#include "ast_node.hpp"

namespace ast {

// (type-name) operand. The conversion is EmitConvert, the same routine
// assignment and calls use (D6). A cast to void evaluates and discards.
class Cast : public Expr {
public:
    Cast(Param type_name, ExprPtr operand)
        : type_(std::move(type_name)), operand_(std::move(operand)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    bool IsNullPointerConstant(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    Param type_;
    ExprPtr operand_;
};

}  // namespace ast
