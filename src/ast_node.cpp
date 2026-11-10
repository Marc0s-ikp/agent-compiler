#include "ast_node.hpp"

#include <stdexcept>

namespace ast {

bool Expr::IsNullPointerConstant(const Context& ctx) const {
    return GetType(ctx)->IsInteger() && EvalConst(ctx) == 0L;
}


std::optional<ConstValue> Expr::EvalConstValue(Context& ctx) const {
    std::optional<long> n = EvalConst(ctx);
    if (!n) return std::nullopt;
    return ConstValue{"", *n};
}

void Expr::EmitAddress(std::ostream&, Context&) const {
    throw std::runtime_error("expression is not an lvalue");
}

}  // namespace ast
