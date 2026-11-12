#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"

namespace ast {

// name(args). Calls to an undeclared function default to returning int (D9).
class FunctionCall : public Expr {
public:
    FunctionCall(std::string name, std::vector<ExprPtr> args)
        : name_(std::move(name)), args_(std::move(args)) {}
    TypePtr GetType(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    // The type each argument is converted to: the declared parameter type if
    // there is one, otherwise the promoted type of the argument itself.
    std::vector<TypePtr> ArgumentTypes(const Context& ctx) const;

    std::string name_;
    std::vector<ExprPtr> args_;
};

}  // namespace ast
