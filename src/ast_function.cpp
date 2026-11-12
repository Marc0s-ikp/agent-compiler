#include "ast_function.hpp"
#include "ir_gen.hpp"
#include "target.hpp"
#include "ast_lower.hpp"

#include <sstream>
#include <stdexcept>

#include "abi.hpp"
#include "emit.hpp"
#include "global_init.hpp"
#include "record_abi.hpp"

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

// Stores one register parameter into its home slot at s0 + home. A word the
// ABI passed in an integer register is stored as an int, even when it is
// the bits of a float or half of a double.
void StoreParamHome(std::ostream& os, const ArgLoc& loc, const TypePtr& type,
                    int home) {
    if (type->IsRecord()) return EmitStoreRecordParam(os, loc, *type, home);
    TypePtr word = Type::Int();
    std::string reg = std::to_string(loc.reg);
    switch (loc.kind) {
        case ArgLoc::Kind::IntReg:
            EmitStore(os, type->IsFloating() ? *word : *type, "a" + reg, "s0", home);
            break;
        case ArgLoc::Kind::FloatReg:
            EmitStore(os, *type, "fa" + reg, "s0", home);
            break;
        case ArgLoc::Kind::IntPair:
            EmitStore(os, *word, "a" + reg, "s0", home);
            EmitStore(os, *word, "a" + std::to_string(loc.reg + 1), "s0", home + 4);
            break;
        case ArgLoc::Kind::Split:
            // The high half is the caller's stack word. Reassembling the two
            // halves with integer stores avoids a misaligned fld.
            EmitStore(os, *word, "a" + reg, "s0", home);
            EmitLoad(os, *word, "t0", "s0", loc.offset);
            EmitStore(os, *word, "t0", "s0", home + 4);
            break;
        case ArgLoc::Kind::Stack:
        case ArgLoc::Kind::Fields: break;  // Fields: structs only
    }
}

// Register parameters are copied to local home slots; after that they are
// ordinary locals. Stack parameters stay where the caller put them.
void EmitParams(std::ostream& os, Context& ctx, const ResolvedParams& params) {
    // A struct returned through a hidden pointer receives it in a0, and the
    // pointer is kept for 'return'.
    bool hidden = ReturnsThroughPointer(*ctx.ReturnType());
    if (hidden) {
        int slot = ctx.AllocLocal(Type::Int());
        EmitStore(os, *Type::Int(), "a0", "s0", slot);
        ctx.SetReturnPointerSlot(slot);
    }
    std::vector<ArgLoc> locs = ClassifyArgs(params.types, hidden);
    for (size_t i = 0; i < params.types.size(); ++i) {
        const TypePtr& type = params.types[i];
        if (locs[i].kind == ArgLoc::Kind::Stack && !locs[i].by_reference) {
            ctx.Declare(params.names[i], Symbol::Local(type, locs[i].offset));
            continue;
        }
        int home = ctx.AllocLocal(type);
        ctx.Declare(params.names[i], Symbol::Local(type, home));
        StoreParamHome(os, locs[i], type, home);
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
    // The name is in scope from the end of its declarator, so the initialiser
    // may use it. An unsized array is completed afterwards by merging.
    ctx.DeclareGlobal(name, type, false);
    MarkLinkage(name, ctx);
    GlobalInit built = BuildGlobalInit(type, *init, ctx);
    ctx.DeclareGlobal(name, built.type, false);
    if (ctx.Lookup(name).type->Size() != built.type->Size()) {
        throw std::runtime_error("conflicting declaration of '" + name + "'");
    }
    ctx.InitGlobal(name, built.data, built.items);
}

// Both paths do the same work here: the global table (merging tentative and
// extern declarations) is in the Context, and each path writes it out at the
// end (EmitGlobals, or ExportGlobals as data entries).
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

void GlobalDeclaration::EmitRISC(std::ostream&, Context& ctx) const { Declare(ctx); }

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
    ctx.EmitFloats(os);
    ctx.PopScope();
}


// ---- --ir lowering ----

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
    if (!type->Base()->IsVoid()) RequireIrScalar(*type->Base());
    ctx.DeclareFunction(name, type);
    ctx.BeginFunction(type->Base());
    gen.BeginFunction(name, storage_ != Storage::Static, type->Base());
    ctx.PushScope();
    for (size_t i = 0; i < params.types.size(); ++i) {
        RequireIrScalar(*params.types[i]);
        ir::Operand home = gen.AddParam(params.names[i], params.types[i]);
        ctx.Declare(params.names[i], Symbol::IrLocal(params.types[i], home.name));
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
