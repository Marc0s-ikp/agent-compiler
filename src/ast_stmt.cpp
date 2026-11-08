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
    if (type_->IsVoid()) throw std::runtime_error("variable of type void");
    for (const InitDeclarator& d : declarators_) {
        int offset = ctx.AllocLocal(type_);
        ctx.Declare(d.name, {type_, offset});
        if (!d.init) continue;
        d.init->EmitRISC(os, ctx);
        EmitConvert(os, *d.init->GetType(ctx), *type_);
        EmitStore(os, *type_, "a0", "s0", offset);
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
