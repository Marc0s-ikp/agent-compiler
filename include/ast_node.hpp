#pragma once
#include <memory>
#include <optional>
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
    // The type of the value this expression yields once evaluated: arrays
    // decay to pointers (D10). Everything that consumes a value uses this;
    // GetType is for the object itself (assignment targets, address-of).
    TypePtr ValueType(const Context& ctx) const { return Decay(GetType(ctx)); }
    // The value if this is an integer constant expression, else nullopt.
    virtual std::optional<long> EvalConst(const Context&) const {
        return std::nullopt;
    }
    // For lvalues: leave the object's address in a0 (D4).
    // Default: the expression is not an lvalue.
    virtual void EmitAddress(std::ostream& os, Context& ctx) const;
};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

}  // namespace ast
