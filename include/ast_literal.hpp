#pragma once
#include <cstdint>
#include <string>

#include "ast_node.hpp"

namespace ast {

class IntLiteral : public Expr {
public:
    // 'text' is the source spelling: decimal, octal or hex, optional u/l suffix.
    explicit IntLiteral(const std::string& text);
    TypePtr GetType(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    uint32_t value_;
    TypePtr type_;
};

}  // namespace ast
