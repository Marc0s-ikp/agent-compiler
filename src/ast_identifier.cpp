#include "ast_identifier.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

#include "emit.hpp"

namespace ast {

namespace {
const Symbol& LookupName(const Context& ctx, const std::string& name) {
    const Symbol& sym = ctx.Lookup(name);
    if (sym.kind == SymbolKind::Function || sym.kind == SymbolKind::Typedef) {
        throw std::runtime_error("'" + name + "' is not a variable");
    }
    return sym;
}
}  // namespace

TypePtr Identifier::GetType(const Context& ctx) const {
    return LookupName(ctx, name_).type;
}

// An enumeration constant is an integer constant expression.
std::optional<long> Identifier::EvalConst(const Context& ctx) const {
    const Symbol* sym = ctx.Find(name_);
    if (sym && sym->kind == SymbolKind::EnumConst) return sym->value;
    return std::nullopt;
}

// Only a global has a constant address. An array's value is that address.
std::optional<ConstValue> Identifier::EvalConstAddress(
    Context& ctx) const {
    const Symbol& sym = LookupName(ctx, name_);
    if (sym.kind != SymbolKind::Global) return std::nullopt;
    return ConstValue{sym.label, 0};
}

std::optional<ConstValue> Identifier::EvalConstValue(
    Context& ctx) const {
    if (std::optional<long> n = EvalConst(ctx)) return ConstValue{"", *n};
    if (!GetType(ctx)->IsArray()) return std::nullopt;
    return EvalConstAddress(ctx);
}

void Identifier::EmitRISC(std::ostream& os, Context& ctx) const {
    const Symbol& sym = LookupName(ctx, name_);
    if (sym.kind == SymbolKind::EnumConst) {
        os << "\tli a0, " << sym.value << "\n";
    } else if (sym.kind == SymbolKind::Global) {
        EmitAddress(os, ctx);
        EmitLoad(os, *sym.type, AccReg(*sym.type), "a0", 0);
    } else {
        EmitLoad(os, *sym.type, AccReg(*sym.type), "s0", sym.offset);
    }
}

void Identifier::EmitAddress(std::ostream& os, Context& ctx) const {
    const Symbol& sym = LookupName(ctx, name_);
    if (sym.kind == SymbolKind::EnumConst) {
        throw std::runtime_error("enumeration constant is not an lvalue");
    }
    if (sym.kind == SymbolKind::Global) {
        os << "\tla a0, " << sym.label << "\n";
        return;
    }
    EmitAddrOffset(os, "a0", "s0", sym.offset);
}


// ---- --ir lowering ----

using ir::Operand;

namespace {
const Symbol& IrVariable(const Context& ctx, const std::string& name) {
    const Symbol& sym = ctx.Lookup(name);
    if (sym.kind == SymbolKind::Global) throw std::runtime_error("not in IR yet: global variable");
    return sym;
}
}  // namespace

Operand Identifier::EmitIRAddress(IRGen& gen) const {
    const Symbol& sym = IrVariable(gen.Ctx(), name_);
    if (sym.kind != SymbolKind::Local) throw std::runtime_error("'" + name_ + "' is not a variable");
    return gen.AddressOf(Operand::Object(sym.object));
}

Operand Identifier::EmitValue(IRGen& gen) const {
    const Symbol& sym = IrVariable(gen.Ctx(), name_);
    if (sym.kind == SymbolKind::EnumConst) return Operand::Int(sym.value, IRGen::ToIr(*sym.type));
    RequireIrInteger(*sym.type);
    return gen.Load(IRGen::ToIr(*sym.type), EmitIRAddress(gen));
}

}  // namespace ast
