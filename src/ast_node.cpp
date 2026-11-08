#include "ast_node.hpp"

#include <stdexcept>

namespace ast {

void Expr::EmitAddress(std::ostream&, Context&) const {
    throw std::runtime_error("expression is not an lvalue");
}

}  // namespace ast
