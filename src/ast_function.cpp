#include "ast_function.hpp"

#include <sstream>

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

}  // namespace

// Register parameters are copied to local home slots; after that they are
// ordinary locals. Stack parameters stay where the caller put them.
void FunctionDefinition::EmitParams(std::ostream& os, Context& ctx) const {
    std::vector<std::string> names;
    std::vector<TypePtr> types;
    for (const Param& p : params_) {
        auto [name, type] = p.declarator.Apply(p.spec.Resolve(ctx), ctx);
        names.push_back(name);
        types.push_back(type);
    }
    std::vector<ArgLoc> locs = ClassifyArgs(types);
    for (size_t i = 0; i < params_.size(); ++i) {
        if (locs[i].kind == ArgLoc::Kind::Stack) {
            ctx.Declare(names[i], {types[i], locs[i].offset});
            continue;
        }
        int home = ctx.AllocLocal(types[i]);
        ctx.Declare(names[i], {types[i], home});
        EmitStore(os, *types[i], "a" + std::to_string(locs[i].reg), "s0", home);
    }
}

// The body is generated first into a buffer, because the frame size it
// needs for the prologue is only known afterwards (D2).
void FunctionDefinition::EmitRISC(std::ostream& os, Context& ctx) const {
    auto [name, type] = declarator_.Apply(return_spec_.Resolve(ctx), ctx);
    ctx.BeginFunction(type);
    std::ostringstream body;
    ctx.PushScope();
    EmitParams(body, ctx);
    body_->EmitRISC(body, ctx);
    ctx.PopScope();

    os << "\t.text\n\t.globl " << name << "\n" << name << ":\n";
    EmitPrologue(os, ctx.FrameSize());
    os << body.str();
    os << ctx.ReturnLabel() << ":\n";
    EmitEpilogue(os);
}

void TranslationUnit::EmitRISC(std::ostream& os, Context& ctx) const {
    for (const ExternalDeclPtr& decl : decls_) decl->EmitRISC(os, ctx);
}

}  // namespace ast
