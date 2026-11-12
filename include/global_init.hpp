#pragma once
#include <string>
#include <vector>

#include "ast_declaration.hpp"
#include "ast_node.hpp"

namespace ast {

// The assembly data directives for a global's initialiser, and the object's
// type: an array declared without a length takes it from the initialiser.
struct GlobalInit {
    TypePtr type;
    std::string data;                 // the old path: assembler directives
    std::vector<ir::DataItem> items;  // the IR path: the same bytes as data items
};

// Every value must be a link-time constant: an integer constant expression,
// or an address constant (label plus offset, Expr::EvalConstValue, which may
// add a string literal to the pool, so the context is mutable). Brace
// lists may be nested, partial (the rest is zero) or flat (brace elision).
// Throws a std::runtime_error if the initialiser is not constant.
GlobalInit BuildGlobalInit(const TypePtr& type, const Initializer& init,
                           Context& ctx);

}  // namespace ast
