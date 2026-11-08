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
        if (type->IsVoid()) throw std::runtime_error("variable of type void");
        int offset = ctx.AllocLocal(type);
        ctx.Declare(name, {type, offset});
        if (!d.init) continue;
        const Expr& value = d.init->Value();
        value.EmitRISC(os, ctx);
        EmitConvert(os, *value.GetType(ctx), *type);
        EmitStore(os, *type, "a0", "s0", offset);
    }
}

void ReturnStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    if (value_) {
        value_->EmitRISC(os, ctx);
        EmitConvert(os, *value_->GetType(ctx), *ctx.ReturnType());
    }
    os << "\tj " << ctx.ReturnLabel() << "\n";
}

}  // namespace ast
