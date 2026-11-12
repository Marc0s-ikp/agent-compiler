#include "ast_function.hpp"
#include "ir_gen.hpp"
#include "target.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

#include "global_init.hpp"

namespace ast {

void GlobalDeclaration::MarkLinkage(const std::string& name,
                                    Context& ctx) const {
    if (storage_ == Storage::Static) ctx.MakeGlobalInternal(name);
}

void GlobalDeclaration::DeclareVariable(const std::string& name,
                                        const TypePtr& type,
                                        const Initializer* init,
                                        Context& ctx) const {
    if (type->IsVoid()) {
        throw std::runtime_error("invalid type for variable '" + name + "'");
    }
    if (!init) {
        ctx.DeclareGlobal(name, type, storage_ == Storage::Extern);
        MarkLinkage(name, ctx);
        return;
    }
    // The name is in scope from the end of its declarator, so the initialiser
    // may use it. An unsized array is completed afterwards by merging.
    ctx.DeclareGlobal(name, type, false);
    MarkLinkage(name, ctx);
    GlobalInit built = BuildGlobalInit(type, *init, ctx);
    ctx.DeclareGlobal(name, built.type, false);
    if (ctx.Lookup(name).type->Size() != built.type->Size()) {
        throw std::runtime_error("conflicting declaration of '" + name + "'");
    }
    ctx.InitGlobal(name, built.items);
}

// The global table (merging tentative and extern declarations) is in the
// Context; ExportGlobals writes it out as data entries at the end.
void GlobalDeclaration::Declare(Context& ctx) const {
    TypePtr base = spec_.Resolve(ctx);
    for (const InitDeclarator& d : declarators_) {
        auto [name, type] = d.declarator.Apply(base, ctx);
        if (storage_ == Storage::Typedef) {
            ctx.Declare(name, Symbol::Typedef(type));
        } else if (!type->IsFunction()) {
            DeclareVariable(name, type, d.init.get(), ctx);
        } else if (d.init) {
            throw std::runtime_error("function '" + name + "' initialised");
        } else {
            ctx.DeclareFunction(name, type);
        }
    }
}

void GlobalDeclaration::EmitIR(IRGen& gen) const { Declare(gen.Ctx()); }

// Each parameter gets a frame object as its home; the backend fills it from
// the classifier's pieces (21.5), so the frontend does not know where the
// arguments arrive.
void FunctionDefinition::EmitIR(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    auto [name, type] = declarator_.Apply(spec_.Resolve(ctx), ctx);
    if (!type->IsFunction()) throw std::runtime_error("not a function");
    const std::vector<Param>* declared = declarator_.Parameters();
    if (!declared) throw std::runtime_error("'" + name + "' is not a function");
    ResolvedParams params = ResolveParams(*declared, ctx);
    if (!type->Base()->IsVoid()) RequireIrValue(*type->Base());
    ctx.DeclareFunction(name, type);
    ctx.BeginFunction(type->Base());
    gen.BeginFunction(name, storage_ != Storage::Static, type->Base());
    ctx.PushScope();
    for (size_t i = 0; i < params.types.size(); ++i) {
        RequireIrValue(*params.types[i]);
        ir::Operand home = gen.AddParam(params.names[i], params.types[i]);
        ctx.Declare(params.names[i], Symbol::Local(params.types[i], home.name));
    }
    body_->EmitIR(gen);
    ctx.PopScope();
    gen.EndFunction();
}

void TranslationUnit::EmitIR(IRGen& gen) const {
    gen.Ctx().PushScope();
    for (const ExternalDeclPtr& decl : decls_) decl->EmitIR(gen);
    gen.Ctx().PopScope();
    gen.EndModule();
}

}  // namespace ast
