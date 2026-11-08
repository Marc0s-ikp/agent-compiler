#pragma once
#include <memory>
#include <ostream>

#include "context.hpp"
#include "type.hpp"

namespace ast {

// Root of the tree. Every node can write its assembly.
class Node {
public:
    virtual ~Node() = default;
    virtual void EmitRISC(std::ostream& os, Context& ctx) const = 0;
};

class Stmt : public Node {};

class Expr : public Node {
public:
    // The type of this expression. Pure: emits nothing (D3).
    virtual TypePtr GetType(const Context& ctx) const = 0;
    // For lvalues: leave the object's address in a0 (D4).
    // Default: the expression is not an lvalue.
    virtual void EmitAddress(std::ostream& os, Context& ctx) const;
};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

}  // namespace ast
