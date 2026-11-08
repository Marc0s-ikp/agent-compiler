#include "ast_identifier.hpp"

#include "emit.hpp"

namespace ast {

TypePtr Identifier::GetType(const Context& ctx) const {
    return ctx.Lookup(name_).type;
}

void Identifier::EmitRISC(std::ostream& os, Context& ctx) const {
    const Symbol& sym = ctx.Lookup(name_);
    EmitLoad(os, *sym.type, "a0", "s0", sym.offset);
}

void Identifier::EmitAddress(std::ostream& os, Context& ctx) const {
    EmitAddrOffset(os, "a0", "s0", ctx.Lookup(name_).offset);
}

}  // namespace ast
