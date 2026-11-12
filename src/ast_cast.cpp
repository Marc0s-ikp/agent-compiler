#include "ast_cast.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

#include "const_value.hpp"
#include "emit.hpp"

namespace ast {

// Only scalar types and void can be cast to.
TypePtr Cast::GetType(const Context& ctx) const {
    TypePtr to = type_.declarator.Apply(type_.spec.Lookup(ctx), ctx).second;
    if (!to->IsVoid() && !to->IsScalar()) {
        throw std::runtime_error("cast to a non-scalar type");
    }
    TypePtr from = operand_->ValueType(ctx);
    if (!to->IsVoid() && !from->IsScalar()) {
        throw std::runtime_error("cast of a non-scalar value");
    }
    return to;
}

// An integer constant expression: an integer constant cast to an integer type.
std::optional<long> Cast::EvalConst(const Context& ctx) const {
    TypePtr to = GetType(ctx);
    if (!to->IsInteger()) return std::nullopt;
    std::optional<long> n = operand_->EvalConst(ctx);
    if (!n) return std::nullopt;
    return WrapToType(*n, *to);
}

bool Cast::IsNullPointerConstant(const Context& ctx) const {
    TypePtr to = GetType(ctx);
    if (to->IsInteger()) return Expr::IsNullPointerConstant(ctx);
    return to->IsPointer() && to->Base()->IsVoid() &&
           operand_->IsNullPointerConstant(ctx);
}

std::optional<ConstValue> Cast::EvalConstValue(Context& ctx) const {
    TypePtr to = GetType(ctx);
    if (to->IsVoid()) return std::nullopt;
    return ConstOperandAs(ctx, *operand_, *to);
}

void Cast::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr to = GetType(ctx);
    operand_->EmitRISC(os, ctx);
    if (!to->IsVoid()) EmitConvert(os, *operand_->ValueType(ctx), *to);
}


// ---- --ir lowering ----

using ir::Operand;

Operand Cast::EmitValue(IRGen& gen) const {
    TypePtr to = GetType(gen.Ctx());
    RequireIrScalar(*to);
    return ValueAs(gen, *operand_, *to);
}

}  // namespace ast
