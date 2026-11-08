#include "ast_stmt.hpp"

#include <stdexcept>

#include "emit.hpp"

namespace ast {

void CompoundStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    ctx.PushScope();
    for (const StmtPtr& stmt : body_) stmt->EmitRISC(os, ctx);
    ctx.PopScope();
}

void ExprStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    if (expr_) expr_->EmitRISC(os, ctx);
}

void DeclarationStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr base = spec_.Resolve(ctx);
    for (const InitDeclarator& d : declarators_) {
        auto [name, type] = d.declarator.Apply(base, ctx);
        if (type->IsVoid() || type->IsFunction()) {
            throw std::runtime_error("invalid type for variable '" + name + "'");
        }
        if (type->Size() == 0) throw std::runtime_error("array size missing");
        int offset = ctx.AllocLocal(type);
        ctx.Declare(name, Symbol::Local(type, offset));
        if (!d.init) continue;
        const Expr& value = d.init->Value();
        value.EmitRISC(os, ctx);
        EmitConvert(os, *value.ValueType(ctx), *type);
        EmitStore(os, *type, "a0", "s0", offset);
    }
}

void ReturnStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    if (value_) {
        value_->EmitRISC(os, ctx);
        EmitConvert(os, *value_->ValueType(ctx), *ctx.ReturnType());
    }
    os << "\tj " << ctx.ReturnLabel() << "\n";
}

}  // namespace ast
