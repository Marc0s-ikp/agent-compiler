#pragma once
#include <string>

#include "ast_node.hpp"

namespace ast {

// A variable or parameter: a local addressed from s0, or a global addressed
// by its label.
class Identifier : public Expr {
public:
    explicit Identifier(std::string name) : name_(std::move(name)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstAddress(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitAddress(std::ostream& os, Context& ctx) const override;

private:
    std::string name_;
};

}  // namespace ast
