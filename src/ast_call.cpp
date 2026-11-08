#include "ast_call.hpp"

#include <stdexcept>

#include "abi.hpp"
#include "ast_operators.hpp"
#include "emit.hpp"

namespace ast {

namespace {

// Bytes of outgoing stack-argument area needed by these locations.
int OutgoingBytes(const std::vector<TypePtr>& types,
                  const std::vector<ArgLoc>& locs) {
    int bytes = 0;
    for (size_t i = 0; i < locs.size(); ++i) {
        if (locs[i].kind != ArgLoc::Kind::Stack) continue;
        int end = locs[i].offset + (types[i]->Size() + 3) / 4 * 4;
        if (end > bytes) bytes = end;
    }
    return bytes;
}

}  // namespace

TypePtr FunctionCall::GetType(const Context& ctx) const {
    const Symbol* sym = ctx.Find(name_);
    if (!sym) return Type::Int();
    if (sym->kind != SymbolKind::Function) {
        throw std::runtime_error("'" + name_ + "' is not a function");
    }
    return sym->type;
}

std::vector<TypePtr> FunctionCall::ArgumentTypes(const Context& ctx) const {
    GetType(ctx);  // rejects calling a non-function
    const Symbol* sym = ctx.Find(name_);
    size_t declared = sym ? sym->params.size() : 0;
    if (args_.size() < declared) {
        throw std::runtime_error("too few arguments to '" + name_ + "'");
    }
    std::vector<TypePtr> types;
    for (size_t i = 0; i < args_.size(); ++i) {
        TypePtr own = RequireInteger(args_[i]->GetType(ctx));
        types.push_back(i < declared ? sym->params[i] : Promote(own));
    }
    return types;
}

// Every argument is evaluated into its own temp slot first, because an
// argument may itself contain a call. Only then are they moved to their
// registers or outgoing stack slots, immediately before the call.
void FunctionCall::EmitRISC(std::ostream& os, Context& ctx) const {
    std::vector<TypePtr> types = ArgumentTypes(ctx);
    std::vector<int> slots;
    for (size_t i = 0; i < args_.size(); ++i) {
        args_[i]->EmitRISC(os, ctx);
        EmitConvert(os, *args_[i]->GetType(ctx), *types[i]);
        slots.push_back(ctx.PushTemp(types[i]));
        EmitStore(os, *types[i], "a0", "s0", slots[i]);
    }

    std::vector<ArgLoc> locs = ClassifyArgs(types);
    ctx.ReserveOutgoing(OutgoingBytes(types, locs));
    for (size_t i = 0; i < args_.size(); ++i) {
        if (locs[i].kind == ArgLoc::Kind::IntReg) {
            EmitLoad(os, *types[i], "a" + std::to_string(locs[i].reg), "s0",
                     slots[i]);
        } else {
            EmitLoad(os, *types[i], "t0", "s0", slots[i]);
            EmitStore(os, *types[i], "t0", "sp", locs[i].offset);
        }
    }
    os << "\tcall " << name_ << "\n";
    for (size_t i = 0; i < args_.size(); ++i) ctx.PopTemp();
}

}  // namespace ast
