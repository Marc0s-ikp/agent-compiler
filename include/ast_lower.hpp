#pragma once
#include "ast_node.hpp"
#include "ast_operators.hpp"
#include "ir_gen.hpp"

// Lowering helpers shared by several nodes (D23 a): the per-node EmitIR and
// EmitValue functions live next to each node's EmitRISC.
namespace ast {

// Throws "not in IR yet" unless 'type' is an integer type.
const Type& RequireIrInteger(const Type& type);

// Throws "not in IR yet" unless 'type' is an integer or a pointer: the types
// a value can have while it is only moved (stored, passed, returned).
const Type& RequireIrScalar(const Type& type);

// a <op> b, both already converted to 'common'.
ir::Operand Arith(IRGen& gen, BinOp op, const Type& common, ir::Operand a, ir::Operand b);

// Converts an already evaluated scalar value between two C scalar types
// (integer, pointer or floating). The one place integers, pointers and
// floats meet (21.10).
ir::Operand ConvertScalar(IRGen& gen, ir::Operand value, const Type& from, const Type& to);

// The conversions with a floating type on either side (ConvertScalar calls it).
ir::Operand ConvertFloating(IRGen& gen, ir::Operand value, const Type& from, const Type& to);

// "x != 0.0" for a floating value, as an I32 0 or 1 (true for NaN).
ir::Operand FloatCondition(IRGen& gen, ir::Operand value);

// Evaluates 'e' and converts it to 'to' (a scalar type).
ir::Operand ValueAs(IRGen& gen, const Expr& e, const Type& to);

// Evaluates a controlling expression (if, loops, ?:, &&, ||, switch) to an
// integer operand that a branch can test against zero. A pointer becomes its
// address as an integer.
ir::Operand Condition(IRGen& gen, const Expr& e);

// The value of the object of C type 'type' at 'address'. An array has no value
// but its address (D10), so it is not loaded; anything else is loaded with its
// IR type (a narrow integer is extended later, by the C type, when it is used).
ir::Operand LoadObject(IRGen& gen, ir::Operand address, const Type& type);

// Pointer arithmetic (21.10). 'pointer' is an evaluated Ptr of C type
// 'pointer_type'; 'index' an evaluated integer of C type 'index_type'.
// Result: pointer +/- index * element size, as a ptradd.
ir::Operand PointerOffset(IRGen& gen, ir::Operand pointer, const Type& pointer_type,
                          ir::Operand index, const Type& index_type, bool subtract);

// (a - b) / element size, as an int: both pointers have C type 'pointer_type'.
ir::Operand PointerDifference(IRGen& gen, ir::Operand a, ir::Operand b, const Type& pointer_type);

}  // namespace ast
