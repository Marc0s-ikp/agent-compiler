#pragma once
#include <string>

#include "ast_node.hpp"

namespace ast {

// A variable or parameter. Both are locals addressed from s0.
class Identifier : public Expr {
public:
    explicit Identifier(std::string name) : name_(std::move(name)) {}
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitAddress(std::ostream& os, Context& ctx) const override;

private:
    std::string name_;
};

}  // namespace ast
