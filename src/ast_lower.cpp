// Lowering helpers shared by several nodes' EmitIR / EmitValue (D23 a). The
// lowering of each node lives next to its EmitRISC; only what more than one
// node needs is here.
#include "ast_lower.hpp"

#include <stdexcept>

namespace ast {

using ir::Op;
using ir::Operand;

const Type& RequireIrInteger(const Type& type) {
    if (!type.IsInteger()) throw std::runtime_error("not in IR yet: non-integer operand");
    return type;
}

namespace {

Op ArithOp(BinOp op, bool is_signed) {
    switch (op) {
        case BinOp::Add: return Op::Add;
        case BinOp::Sub: return Op::Sub;
        case BinOp::Mul: return Op::Mul;
        case BinOp::Div: return is_signed ? Op::Div : Op::DivU;
        case BinOp::Mod: return is_signed ? Op::Rem : Op::RemU;
        case BinOp::Shl: return Op::Shl;
        case BinOp::Shr: return is_signed ? Op::Sar : Op::Shr;
        case BinOp::And: return Op::And;
        case BinOp::Or: return Op::Or;
        case BinOp::Xor: return Op::Xor;
    }
    return Op::Add;
}

}  // namespace

// a <op> b, both already converted to 'common'.
Operand Arith(IRGen& gen, BinOp op, const Type& common, Operand a, Operand b) {
    return gen.Binary(ArithOp(op, common.IsSigned()), IRGen::ToIr(common), a, b);
}

// Evaluates 'e' and converts it to 'to' (an integer type).
Operand ValueAs(IRGen& gen, const Expr& e, const Type& to) {
    TypePtr from = e.ValueType(gen.Ctx());
    RequireIrInteger(*from);
    return gen.ConvertInt(e.EmitValue(gen), *from, to);
}
 
Operand Condition(IRGen& gen, const Expr& e) {
    RequireIrInteger(*e.ValueType(gen.Ctx()));
    return e.EmitValue(gen);
}

}  // namespace ast
