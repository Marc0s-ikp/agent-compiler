#include "ast_member.hpp"

#include <stdexcept>

#include "ast_lower.hpp"
#include "emit.hpp"

namespace ast {

const RecordMember& Member::Find(const Context& ctx) const {
    TypePtr record = arrow_ ? base_->ValueType(ctx) : base_->GetType(ctx);
    if (arrow_) {
        if (!record->IsPointer()) {
            throw std::runtime_error("'->' applied to a non-pointer");
        }
        record = record->Base();
    }
    if (!record->IsRecord()) {
        throw std::runtime_error("member '" + name_ + "' of a non-struct");
    }
    if (!record->IsComplete()) {
        throw std::runtime_error("member '" + name_ + "' of an incomplete type");
    }
    const RecordMember* member = record->FindMember(name_);
    if (!member) throw std::runtime_error("no member named '" + name_ + "'");
    return *member;
}

TypePtr Member::GetType(const Context& ctx) const { return Find(ctx).type; }

// Only '.' on an object with a constant address: &global.member.
std::optional<ConstValue> Member::EvalConstAddress(Context& ctx) const {
    const RecordMember& member = Find(ctx);
    if (arrow_) return std::nullopt;
    std::optional<ConstValue> base = base_->EvalConstAddress(ctx);
    if (!base) return std::nullopt;
    base->offset += member.offset;
    return base;
}

std::optional<ConstValue> Member::EvalConstValue(Context& ctx) const {
    if (!GetType(ctx)->IsArray()) return std::nullopt;
    return EvalConstAddress(ctx);
}

void Member::EmitAddress(std::ostream& os, Context& ctx) const {
    const RecordMember& member = Find(ctx);
    base_->EmitRISC(os, ctx);
    EmitAddrOffset(os, "a0", "a0", member.offset);
}

// An array or struct member is its own address; anything else is loaded.
void Member::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitAddress(os, ctx);
    TypePtr type = GetType(ctx);
    EmitLoad(os, *type, AccReg(*type), "a0", 0);
}



// ---- --ir lowering ----

// The base's value is the struct's address for '.', and the pointer for '->';
// the member is that address plus its offset (no instruction for offset 0).
ir::Operand Member::EmitIRAddress(IRGen& gen) const {
    const RecordMember& member = Find(gen.Ctx());
    ir::Operand base = base_->EmitValue(gen);
    if (member.offset == 0) return base;
    return gen.PtrAdd(base, ir::Operand::Int(member.offset, ir::PointerIntType()));
}

ir::Operand Member::EmitValue(IRGen& gen) const {
    return LoadObject(gen, EmitIRAddress(gen), *GetType(gen.Ctx()));
}

}  // namespace ast
