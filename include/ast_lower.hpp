#pragma once
#include "ast_node.hpp"
#include "ast_operators.hpp"
#include "ir_gen.hpp"

// Lowering helpers shared by several nodes (D23 a): the per-node EmitIR and
// EmitValue functions live next to each node's EmitRISC.
namespace ast {

// Throws "not in IR yet" unless 'type' is an integer type.
const Type& RequireIrInteger(const Type& type);

// a <op> b, both already converted to 'common'.
ir::Operand Arith(IRGen& gen, BinOp op, const Type& common, ir::Operand a, ir::Operand b);

// Evaluates 'e' and converts it to 'to' (an integer type).
ir::Operand ValueAs(IRGen& gen, const Expr& e, const Type& to);

// Evaluates a controlling expression (if, loops, ?:, &&, ||, switch) to an
// integer operand that a branch can test against zero.
ir::Operand Condition(IRGen& gen, const Expr& e);

}  // namespace ast
