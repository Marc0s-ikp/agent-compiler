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
    std::optional<long> EvalConst(const Context& ctx) const override;
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    // Throws if the value does not fit 32 bits. Checked at code generation,
    // not in the constructor, so the parser never throws.
    void CheckRange() const;

    unsigned long long value_;
    bool has_u_;
    std::string text_;
};

}  // namespace ast
