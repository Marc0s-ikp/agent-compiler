#include "ast_stmt.hpp"

#include <stdexcept>

#include "emit.hpp"
#include "global_init.hpp"

namespace ast {

void CompoundStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    ctx.PushScope();
    for (const StmtPtr& stmt : body_) stmt->EmitRISC(os, ctx);
    ctx.PopScope();
}

void ExprStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    if (expr_) expr_->EmitRISC(os, ctx);
}

// A static local lives in .data/.bss under its own label; its initialiser is
// a compile-time constant, evaluated once like a global's.
void DeclarationStmt::DeclareStatic(const std::string& name, const TypePtr& type,
                                    const Initializer* init,
                                    Context& ctx) const {
    TypePtr final_type = type;
    std::string data;
    if (init) {
        GlobalInit built = BuildGlobalInit(type, *init, ctx);
        final_type = built.type;
        data = built.data;
    }
    if (final_type->Size() == 0) throw std::runtime_error("array size missing");
    std::string label = ctx.AddStaticLocal(final_type, init ? &data : nullptr);
    ctx.Declare(name, Symbol::Global(final_type, label));
}

void DeclarationStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr base = spec_.Resolve(ctx);
    for (const InitDeclarator& d : declarators_) {
        auto [name, type] = d.declarator.Apply(base, ctx);
        if (type->IsVoid() || type->IsFunction()) {
            throw std::runtime_error("invalid type for variable '" + name + "'");
        }
        if (storage_ == Storage::Static) {
            DeclareStatic(name, type, d.init.get(), ctx);
            continue;
        }
        if (type->Size() == 0) throw std::runtime_error("array size missing");
        int offset = ctx.AllocLocal(type);
        ctx.Declare(name, Symbol::Local(type, offset));
        if (!d.init) continue;
        if (d.init->IsList()) {
            throw std::runtime_error("brace initialiser for a local variable");
        }
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
