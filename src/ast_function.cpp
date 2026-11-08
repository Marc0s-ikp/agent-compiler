#include "ast_function.hpp"

#include <sstream>
#include <stdexcept>

#include "abi.hpp"
#include "emit.hpp"

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

// Parameter names and types after applying the declarators. A lone unnamed
// 'void' means "no parameters"; any other void parameter is an error.
struct ResolvedParams {
    std::vector<std::string> names;
    std::vector<TypePtr> types;
};

ResolvedParams ResolveParams(const std::vector<Param>& params, Context& ctx) {
    ResolvedParams out;
    for (const Param& p : params) {
        auto [name, type] = p.declarator.Apply(p.spec.Resolve(ctx), ctx);
        if (type->IsVoid() && (params.size() > 1 || !name.empty())) {
            throw std::runtime_error("parameter of type void");
        }
        if (type->IsVoid()) return {};
        out.names.push_back(name);
        out.types.push_back(type);
    }
    return out;
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

void FunctionDeclaration::EmitRISC(std::ostream&, Context& ctx) const {
    auto [name, type] = declarator_.Apply(return_spec_.Resolve(ctx), ctx);
    ctx.DeclareFunction(name, type, ResolveParams(params_, ctx).types);
}

// The body is generated first into a buffer, because the frame size it
// needs for the prologue is only known afterwards (D2). The function is
// declared before its body so that it can call itself.
void FunctionDefinition::EmitRISC(std::ostream& os, Context& ctx) const {
    auto [name, type] = declarator_.Apply(return_spec_.Resolve(ctx), ctx);
    ResolvedParams params = ResolveParams(params_, ctx);
    ctx.DeclareFunction(name, type, params.types);
    ctx.BeginFunction(type);
    std::ostringstream body;
    ctx.PushScope();
    EmitParams(body, ctx, params);
    body_->EmitRISC(body, ctx);
    ctx.PopScope();

    os << "\t.text\n\t.globl " << name << "\n" << name << ":\n";
    EmitPrologue(os, ctx.FrameSize());
    os << body.str();
    os << ctx.ReturnLabel() << ":\n";
    EmitEpilogue(os);
}

// File scope holds the function symbols.
void TranslationUnit::EmitRISC(std::ostream& os, Context& ctx) const {
    ctx.PushScope();
    for (const ExternalDeclPtr& decl : decls_) decl->EmitRISC(os, ctx);
    ctx.PopScope();
}

}  // namespace ast
