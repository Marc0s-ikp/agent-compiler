#include "ast_call.hpp"

#include <algorithm>
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
        bytes = std::max(bytes, StackExtent(locs[i], *types[i]));
    }
    return bytes;
}

std::string IntArgReg(int n) { return "a" + std::to_string(n); }

// Moves argument 'type', saved in the temp slot at s0 + slot, to its place.
// A word the ABI passes in an integer register is loaded as an int, even
// when it is half of a double or the bits of a float.
void PlaceArg(std::ostream& os, const ArgLoc& loc, const TypePtr& type,
              int slot) {
    TypePtr word = Type::Int();
    switch (loc.kind) {
        case ArgLoc::Kind::IntReg:
            EmitLoad(os, type->IsFloating() ? *word : *type,
                     IntArgReg(loc.reg), "s0", slot);
            break;
        case ArgLoc::Kind::FloatReg:
            EmitLoad(os, *type, "fa" + std::to_string(loc.reg), "s0", slot);
            break;
        case ArgLoc::Kind::IntPair:
            EmitLoad(os, *word, IntArgReg(loc.reg), "s0", slot);
            EmitLoad(os, *word, IntArgReg(loc.reg + 1), "s0", slot + 4);
            break;
        case ArgLoc::Kind::Split:
            EmitLoad(os, *word, IntArgReg(loc.reg), "s0", slot);
            EmitLoad(os, *word, "t0", "s0", slot + 4);
            EmitStore(os, *word, "t0", "sp", loc.offset);
            break;
        case ArgLoc::Kind::Stack:
            EmitLoad(os, *type, ScratchReg(*type), "s0", slot);
            EmitStore(os, *type, ScratchReg(*type), "sp", loc.offset);
            break;
    }
}

}  // namespace

TypePtr FunctionCall::GetType(const Context& ctx) const {
    const Symbol* sym = ctx.Find(name_);
    if (!sym) return Type::Int();
    if (sym->kind != SymbolKind::Function) {
        throw std::runtime_error("'" + name_ + "' is not a function");
    }
    return sym->type->Base();
}

// What an argument with no declared parameter is passed as: integers
// promote to int, and float to double.
static TypePtr DefaultPromote(const TypePtr& t) {
    return t->IsFloating() ? Type::Double() : Promote(t);
}

std::vector<TypePtr> FunctionCall::ArgumentTypes(const Context& ctx) const {
    GetType(ctx);  // rejects calling a non-function
    const Symbol* sym = ctx.Find(name_);
    const std::vector<TypePtr> none;
    const std::vector<TypePtr>& params = sym ? sym->type->Params() : none;
    size_t declared = params.size();
    if (args_.size() < declared) {
        throw std::runtime_error("too few arguments to '" + name_ + "'");
    }
    std::vector<TypePtr> types;
    for (size_t i = 0; i < args_.size(); ++i) {
        TypePtr own = RequireScalar(args_[i]->ValueType(ctx));
        types.push_back(i < declared ? params[i] : DefaultPromote(own));
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
        EmitConvert(os, *args_[i]->ValueType(ctx), *types[i]);
        slots.push_back(ctx.PushTemp(types[i]));
        EmitStore(os, *types[i], AccReg(*types[i]), "s0", slots[i]);
    }

    std::vector<ArgLoc> locs = ClassifyArgs(types);
    ctx.ReserveOutgoing(OutgoingBytes(types, locs));
    for (size_t i = 0; i < args_.size(); ++i) {
        PlaceArg(os, locs[i], types[i], slots[i]);
    }
    os << "\tcall " << name_ << "\n";
    for (size_t i = 0; i < args_.size(); ++i) ctx.PopTemp();
}

}  // namespace ast
