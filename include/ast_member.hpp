#pragma once
#include <string>

#include "ast_node.hpp"

namespace ast {

// base.name or base->name. A struct value is represented by its address
// (D19), so both forms start from the value of 'base' (the object's address
// for '.', the pointer for '->') and add the member's offset.
class Member : public Expr {
public:
    Member(ExprPtr base, std::string name, bool is_arrow)
        : base_(std::move(base)), name_(std::move(name)), arrow_(is_arrow) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    std::optional<ConstValue> EvalConstAddress(Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;
    ir::Operand EmitIRAddress(IRGen& gen) const override;

private:
    // The member found in the base's record type; throws if the base is not
    // a complete struct or union (or pointer to one) with that member.
    const RecordMember& Find(const Context& ctx) const;

    ExprPtr base_;
    std::string name_;
    bool arrow_;
};

}  // namespace ast
