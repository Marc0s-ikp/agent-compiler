#include "ast_stmt.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

#include "global_init.hpp"
#include "local_init.hpp"

namespace ast {

// A static local lives in .data/.bss under its own label; its initialiser is
// a compile-time constant, evaluated once like a global's.
void DeclarationStmt::DeclareStatic(const std::string& name, const TypePtr& type,
                                    const Initializer* init,
                                    Context& ctx) const {
    // Declared before the initialiser is built, so that it can name itself.
    std::string label = ctx.AddStaticLocal(type);
    ctx.Declare(name, Symbol::Global(type, label));
    if (!init) {
        if (type->Size() == 0) throw std::runtime_error("incomplete type or missing array size");
        return;
    }
    GlobalInit built = BuildGlobalInit(type, *init, ctx);
    if (built.type->Size() == 0) throw std::runtime_error("incomplete type or missing array size");
    ctx.CompleteStaticLocal(label, built.type, built.items);
    ctx.Declare(name, Symbol::Global(built.type, label));
}

using ir::Operand;

void CompoundStmt::EmitIR(IRGen& gen) const {
    gen.Ctx().PushScope();
    for (const StmtPtr& stmt : body_) stmt->EmitIR(gen);
    gen.Ctx().PopScope();
}

void ExprStmt::EmitIR(IRGen& gen) const {
    if (expr_) expr_->EmitValue(gen);
}

void DeclarationStmt::EmitIR(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr base = spec_.Resolve(ctx);
    for (const InitDeclarator& d : declarators_) {
        auto [name, type] = d.declarator.Apply(base, ctx);
        if (storage_ == Storage::Typedef) {
            ctx.Declare(name, Symbol::Typedef(type));
            continue;
        }
        if (type->IsVoid() || type->IsFunction()) {
            throw std::runtime_error("invalid type for variable '" + name + "'");
        }
        if (storage_ == Storage::Static) {  // a data entry, no code
            DeclareStatic(name, type, d.init.get(), ctx);
            continue;
        }
        DeclareLocalIR(gen, name, type, d.init.get());
    }
}

// A local is a frame object. The initialiser is laid out first because an
// array declared without a length takes it from there ('char s[] = "abc"').
void DeclarationStmt::DeclareLocalIR(IRGen& gen, const std::string& name, TypePtr type,
                                     const Initializer* init) const {
    Context& ctx = gen.Ctx();
    FlatInit flat;
    if (init) {
        flat = FlattenInit(type, *init, ctx);
        type = flat.type;
    }
    if (type->Size() == 0) throw std::runtime_error("incomplete type or missing array size");
    if (!type->IsArray()) RequireIrValue(*type);
    Operand object = gen.AddFrameObject(name, *type);
    ctx.Declare(name, Symbol::Local(type, object.name));
    if (init) EmitLocalInit(gen, flat, object);
}

void ReturnStmt::EmitIR(IRGen& gen) const {
    if (!value_) return gen.Return();
    const TypePtr& to = gen.Ctx().ReturnType();
    RequireIrValue(*to);
    gen.Return(ValueAs(gen, *value_, *to));
}

}  // namespace ast
