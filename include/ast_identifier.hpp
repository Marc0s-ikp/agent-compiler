#pragma once
#include <string>

#include "ast_node.hpp"

namespace ast {

// A variable or parameter: a local or parameter is a frame object, a global
// is addressed by its label.
class Identifier : public Expr {
public:
    explicit Identifier(std::string name) : name_(std::move(name)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    std::optional<ConstValue> EvalConstAddress(Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;
    ir::Operand EmitIRAddress(IRGen& gen) const override;

private:
    std::string name_;
};

}  // namespace ast
