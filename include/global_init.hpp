#pragma once
#include <string>

#include "ast_declaration.hpp"
#include "ast_node.hpp"

namespace ast {

// The assembly data directives for a global's initialiser, and the object's
// type: an array declared without a length takes it from the initialiser.
struct GlobalInit {
    TypePtr type;
    std::string data;
};

// Every value must be a link-time constant: an integer constant expression,
// or an address constant (label plus offset, Expr::EvalConstValue). Brace
// lists may be nested, partial (the rest is zero) or flat (brace elision).
// Throws a std::runtime_error if the initialiser is not constant.
GlobalInit BuildGlobalInit(const TypePtr& type, const Initializer& init,
                           const Context& ctx);

}  // namespace ast
