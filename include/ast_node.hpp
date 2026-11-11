#pragma once
#include <memory>
#include <optional>
#include <ostream>
#include <string>

#include "context.hpp"
#include "ir.hpp"
#include "type.hpp"

class IRGen;

namespace ast {

// Root of the tree. Every node can write its assembly.
class Node {
public:
    virtual ~Node() = default;
    virtual void EmitRISC(std::ostream& os, Context& ctx) const = 0;
    // The --ir path (D21). Nodes that have no IR lowering yet throw
    // "not in IR yet", so a test using one fails under --ir and still passes
    // on the old path.
    virtual void EmitIR(IRGen& gen) const;
};

// A link-time constant: the address of a global label plus a byte offset, or
// a plain integer when the label is empty (D11, "Address constants" row).
// A floating constant sets is_real and keeps its value in 'real' (D18). The
// flag is set exactly when the expression's type is float or double, and a
// float constant already holds a value that float can represent.
struct ConstValue {
    std::string label;
    long offset = 0;
    bool is_real = false;
    double real = 0;
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
    // True for an integer constant expression of value 0, or such a constant
    // cast to 'void *'. The C rule for what may stand for a null pointer.
    virtual bool IsNullPointerConstant(const Context& ctx) const;
    // The value as a constant that may name a global's address ('&x', 'arr',
    // 'arr + 1'). Defaults to EvalConst, so integer constants need no override.
    // The context is mutable because a string literal's address constant
    // interns the string in the data pool (D17).
    virtual std::optional<ConstValue> EvalConstValue(Context& ctx) const;
    // For lvalues: the constant address of the object (what '&' yields).
    virtual std::optional<ConstValue> EvalConstAddress(Context&) const {
        return std::nullopt;
    }
    // The bytes of a string literal (without the NUL), else null. Lets an
    // initialiser tell 'char s[] = "ab"' from 'char *s = "ab"'.
    virtual const std::string* StringBytes() const { return nullptr; }
    // For lvalues: leave the object's address in a0 (D4).
    // Default: the expression is not an lvalue.
    virtual void EmitAddress(std::ostream& os, Context& ctx) const;
    // --ir: evaluates the expression and returns an operand holding its
    // value, with the IR type of ValueType(ctx).
    virtual ir::Operand EmitValue(IRGen& gen) const;
    // --ir, lvalues: the address of the object, as a Ptr operand.
    virtual ir::Operand EmitIRAddress(IRGen& gen) const;
};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

}  // namespace ast
