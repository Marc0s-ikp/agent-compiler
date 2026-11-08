#include "ast_identifier.hpp"

#include <stdexcept>

#include "emit.hpp"

namespace ast {

namespace {
const Symbol& LookupVariable(const Context& ctx, const std::string& name) {
    const Symbol& sym = ctx.Lookup(name);
    if (sym.kind != SymbolKind::Local) {
        throw std::runtime_error("'" + name + "' is not a variable");
    }
    return sym;
}
}  // namespace

TypePtr Identifier::GetType(const Context& ctx) const {
    return LookupVariable(ctx, name_).type;
}

void Identifier::EmitRISC(std::ostream& os, Context& ctx) const {
    const Symbol& sym = LookupVariable(ctx, name_);
    EmitLoad(os, *sym.type, "a0", "s0", sym.offset);
}

void Identifier::EmitAddress(std::ostream& os, Context& ctx) const {
    EmitAddrOffset(os, "a0", "s0", LookupVariable(ctx, name_).offset);
}

}  // namespace ast
