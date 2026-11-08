#include "ast_node.hpp"

#include <stdexcept>

namespace ast {

std::optional<ConstValue> Expr::EvalConstValue(const Context& ctx) const {
    std::optional<long> n = EvalConst(ctx);
    if (!n) return std::nullopt;
    return ConstValue{"", *n};
}

void Expr::EmitAddress(std::ostream&, Context&) const {
    throw std::runtime_error("expression is not an lvalue");
}

}  // namespace ast
