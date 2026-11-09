#include "ast_sizeof.hpp"

#include <stdexcept>

namespace ast {

namespace {

// sizeof yields size_t, which is unsigned int on this target.
TypePtr SizeType() { return Type::UInt(); }

// The size of an object type; void, functions and incomplete arrays have none.
long ObjectSize(const Type& type) {
    if (type.IsVoid() || type.IsFunction() || type.Size() == 0) {
        throw std::runtime_error("sizeof applied to an incomplete type");
    }
    return type.Size();
}

void EmitSize(std::ostream& os, long size) { os << "\tli a0, " << size << "\n"; }

}  // namespace

// ---- SizeofExpr ----

TypePtr SizeofExpr::GetType(const Context& ctx) const {
    ObjectSize(*operand_->GetType(ctx));
    return SizeType();
}

std::optional<long> SizeofExpr::EvalConst(const Context& ctx) const {
    return ObjectSize(*operand_->GetType(ctx));
}

void SizeofExpr::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitSize(os, ObjectSize(*operand_->GetType(ctx)));
}

// ---- SizeofType ----

namespace {
TypePtr NamedType(const Param& p, const Context& ctx) {
    return p.declarator.Apply(p.spec.Lookup(ctx), ctx).second;
}
}  // namespace

TypePtr SizeofType::GetType(const Context& ctx) const {
    ObjectSize(*NamedType(type_, ctx));
    return SizeType();
}

std::optional<long> SizeofType::EvalConst(const Context& ctx) const {
    return ObjectSize(*NamedType(type_, ctx));
}

void SizeofType::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitSize(os, ObjectSize(*NamedType(type_, ctx)));
}

}  // namespace ast
