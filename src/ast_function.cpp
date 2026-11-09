#include "ast_function.hpp"

#include <sstream>
#include <stdexcept>

#include "abi.hpp"
#include "emit.hpp"
#include "global_init.hpp"

namespace ast {

namespace {

// Allocate the frame, save ra and the caller's s0, point s0 at the old sp.
void EmitPrologue(std::ostream& os, int size) {
    EmitAddrOffset(os, "sp", "sp", -size);
    EmitMem(os, "sw", "ra", "sp", size - 4);
    EmitMem(os, "sw", "s0", "sp", size - 8);
    EmitAddrOffset(os, "s0", "sp", size);
}

void EmitEpilogue(std::ostream& os) {
    EmitMem(os, "lw", "ra", "s0", -4);
    EmitMem(os, "lw", "t0", "s0", -8);
    os << "\tmv sp, s0\n\tmv s0, t0\n\tret\n";
}

// Register parameters are copied to local home slots; after that they are
// ordinary locals. Stack parameters stay where the caller put them.
void EmitParams(std::ostream& os, Context& ctx, const ResolvedParams& params) {
    std::vector<ArgLoc> locs = ClassifyArgs(params.types);
    for (size_t i = 0; i < params.types.size(); ++i) {
        const TypePtr& type = params.types[i];
        if (locs[i].kind == ArgLoc::Kind::Stack) {
            ctx.Declare(params.names[i], Symbol::Local(type, locs[i].offset));
            continue;
        }
        int home = ctx.AllocLocal(type);
        ctx.Declare(params.names[i], Symbol::Local(type, home));
        EmitStore(os, *type, "a" + std::to_string(locs[i].reg), "s0", home);
    }
}

}  // namespace

namespace {

// The declared type must be a function type; returns it with the name.
std::pair<std::string, TypePtr> ResolveFunction(const TypeSpecifier& spec,
                                                const Declarator& declarator,
                                                Context& ctx) {
    auto [name, type] = declarator.Apply(spec.Resolve(ctx), ctx);
    if (!type->IsFunction()) throw std::runtime_error("not a function");
    return {name, type};
}

}  // namespace

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
    GlobalInit built = BuildGlobalInit(type, *init, ctx);
    ctx.DeclareGlobal(name, built.type, false);
    MarkLinkage(name, ctx);
    if (ctx.Lookup(name).type->Size() != built.type->Size()) {
        throw std::runtime_error("conflicting declaration of '" + name + "'");
    }
    ctx.InitGlobal(name, built.data);
}

void GlobalDeclaration::EmitRISC(std::ostream&, Context& ctx) const {
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

// The body is generated first into a buffer, because the frame size it
// needs for the prologue is only known afterwards (D2). The function is
// declared before its body so that it can call itself.
void FunctionDefinition::EmitRISC(std::ostream& os, Context& ctx) const {
    auto [name, type] = ResolveFunction(spec_, declarator_, ctx);
    const std::vector<Param>* declared = declarator_.Parameters();
    if (!declared) throw std::runtime_error("'" + name + "' is not a function");
    ResolvedParams params = ResolveParams(*declared, ctx);
    ctx.DeclareFunction(name, type);
    ctx.BeginFunction(type->Base());
    std::ostringstream body;
    ctx.PushScope();
    EmitParams(body, ctx, params);
    body_->EmitRISC(body, ctx);
    ctx.PopScope();

    os << "\t.text\n";
    if (storage_ != Storage::Static) os << "\t.globl " << name << "\n";
    os << name << ":\n";
    EmitPrologue(os, ctx.FrameSize());
    os << body.str();
    os << ctx.ReturnLabel() << ":\n";
    EmitEpilogue(os);
}

// File scope holds the function and global symbols.
void TranslationUnit::EmitRISC(std::ostream& os, Context& ctx) const {
    ctx.PushScope();
    for (const ExternalDeclPtr& decl : decls_) decl->EmitRISC(os, ctx);
    ctx.EmitGlobals(os);
    ctx.EmitStrings(os);
    ctx.PopScope();
}

}  // namespace ast
