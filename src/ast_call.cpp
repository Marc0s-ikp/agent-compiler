#include "ast_call.hpp"

#include <algorithm>
#include <stdexcept>

#include "abi.hpp"
#include "ast_operators.hpp"
#include "emit.hpp"
#include "record_abi.hpp"

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
    if (type->IsRecord()) return EmitPlaceRecordArg(os, loc, *type, slot);
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
        case ArgLoc::Kind::Fields: break;  // structs only
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
        TypePtr own = args_[i]->ValueType(ctx);
        if (!own->IsRecord()) RequireScalar(own);
        types.push_back(i < declared ? params[i] : DefaultPromote(own));
    }
    return types;
}

// Saves the argument value (in the accumulator, or for a struct the address
// of it) into its temp slot. A struct is copied, so the callee gets a value
// that later arguments cannot change.
void SaveArg(std::ostream& os, Context& ctx, const TypePtr& type, int& slot) {
    slot = ctx.PushTemp(type);
    if (!type->IsRecord()) return EmitStore(os, *type, AccReg(*type), "s0", slot);
    EmitAddrOffset(os, "t1", "s0", slot);
    EmitCopyMem(os, "t1", "a0", type->Size(), type->Align());
}

// Every argument is evaluated into its own temp slot first, because an
// argument may itself contain a call. Only then are they moved to their
// registers or outgoing stack slots, immediately before the call.
//
// A struct result is written into a slot of the function's own frame that
// lives until the function ends (D19), so it survives the next call that
// pushes and pops temps: g(f(1), f(2)) keeps both.
void FunctionCall::EmitRISC(std::ostream& os, Context& ctx) const {
    std::vector<TypePtr> types = ArgumentTypes(ctx);
    TypePtr result_type = GetType(ctx);
    bool is_record = result_type->IsRecord();
    bool hidden = is_record && ReturnsThroughPointer(*result_type);
    int result = is_record ? ctx.AllocLocal(result_type) : 0;
    std::vector<int> slots(args_.size());
    for (size_t i = 0; i < args_.size(); ++i) {
        args_[i]->EmitRISC(os, ctx);
        EmitConvert(os, *args_[i]->ValueType(ctx), *types[i]);
        SaveArg(os, ctx, types[i], slots[i]);
    }

    std::vector<ArgLoc> locs = ClassifyArgs(types, hidden);
    ctx.ReserveOutgoing(OutgoingBytes(types, locs));
    for (size_t i = 0; i < args_.size(); ++i) {
        PlaceArg(os, locs[i], types[i], slots[i]);
    }
    if (hidden) EmitAddrOffset(os, "a0", "s0", result);
    os << "\tcall " << name_ << "\n";
    if (is_record) EmitSaveReturnedRecord(os, *result_type, result);
    for (size_t i = 0; i < args_.size(); ++i) ctx.PopTemp();
}

}  // namespace ast
